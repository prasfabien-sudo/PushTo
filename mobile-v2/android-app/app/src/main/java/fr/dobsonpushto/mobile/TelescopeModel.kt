package fr.dobsonpushto.mobile

import android.content.Context
import org.json.JSONArray
import org.json.JSONObject
import kotlin.math.abs
import kotlin.math.roundToInt

data class EncoderSnapshot(val azTicks: Int, val altTicks: Int, val azDegrees: Double, val altDegrees: Double)

class TelescopeModel(context: Context) {
    private val preferences = context.getSharedPreferences("dobson-mobile", Context.MODE_PRIVATE)
    private val defaults = linkedMapOf(
        "Chavanod" to ObservationSite("Chavanod", 45.89, 6.05),
        "Annecy" to ObservationSite("Annecy", 45.90, 6.12),
        "Lyon" to ObservationSite("Lyon", 45.76, 4.83),
        "Paris" to ObservationSite("Paris", 48.85, 2.35)
    )

    @Volatile var rawAzTicks: Int = 0
        private set
    @Volatile var rawAltTicks: Int = 0
        private set
    @Volatile var simulationAzTicks: Int = 0
        private set
    @Volatile var simulationAltTicks: Int = 0
        private set
    @Volatile var connected: Boolean = false
    @Volatile var lastTelemetryAt: Long = 0
        private set

    var ticksPerRevAz: Double
        get() = preferences.getFloat("ticksAZ", 10000.0f).toDouble()
        set(value) { preferences.edit().putFloat("ticksAZ", value.toFloat()).apply() }
    var ticksPerRevAlt: Double
        get() = preferences.getFloat("ticksALT", 10000.0f).toDouble()
        set(value) { preferences.edit().putFloat("ticksALT", value.toFloat()).apply() }
    var reverseAz: Boolean
        get() = preferences.getBoolean("revAZ", false)
        set(value) { preferences.edit().putBoolean("revAZ", value).apply() }
    var reverseAlt: Boolean
        get() = preferences.getBoolean("revALT", false)
        set(value) { preferences.edit().putBoolean("revALT", value).apply() }
    var nightMode: Boolean
        get() = preferences.getBoolean("nightMode", false)
        set(value) { preferences.edit().putBoolean("nightMode", value).apply() }
    var siteName: String
        get() = preferences.getString("siteName", "Chavanod") ?: "Chavanod"
        private set(value) { preferences.edit().putString("siteName", value).apply() }
    var latitude: Double
        get() = preferences.getFloat("latitude", 45.89f).toDouble()
        private set(value) { preferences.edit().putFloat("latitude", value.toFloat()).apply() }
    var longitude: Double
        get() = preferences.getFloat("longitude", 6.05f).toDouble()
        private set(value) { preferences.edit().putFloat("longitude", value.toFloat()).apply() }
    var azOffsetDegrees: Double
        get() = preferences.getFloat("azOffset", 0.0f).toDouble()
        private set(value) { preferences.edit().putFloat("azOffset", value.toFloat()).apply() }
    var altOffsetDegrees: Double
        get() = preferences.getFloat("altOffset", 0.0f).toDouble()
        private set(value) { preferences.edit().putFloat("altOffset", value.toFloat()).apply() }
    var selectedTargetName: String
        get() = preferences.getString("targetName", "") ?: ""
        private set(value) { preferences.edit().putString("targetName", value).apply() }

    var activeTarget: SkyObject? = null
        private set

    init {
        activeTarget = null
    }

    fun acceptTelemetry(azTicks: Int, altTicks: Int) {
        rawAzTicks = azTicks
        rawAltTicks = altTicks
        lastTelemetryAt = System.currentTimeMillis()
    }

    fun snapshot(): EncoderSnapshot {
        val rawAz = (rawAzTicks + simulationAzTicks).toDouble() * if (reverseAz) -1.0 else 1.0
        val rawAlt = (rawAltTicks + simulationAltTicks).toDouble() * if (reverseAlt) -1.0 else 1.0
        val az = Astronomy.positiveModulo(rawAz / safeResolution(ticksPerRevAz) * 360.0 + azOffsetDegrees, 360.0)
        val altCircle = Astronomy.positiveModulo(
            rawAlt / safeResolution(ticksPerRevAlt) * 360.0 + altOffsetDegrees, 360.0
        )
        val altitude = if (altCircle > 180.0) altCircle - 360.0 else altCircle
        return EncoderSnapshot(rawAzTicks, rawAltTicks, az, altitude)
    }

    fun setZeroAzimuth() {
        val current = snapshot().azDegrees
        azOffsetDegrees -= current
        persistOffsets()
    }

    fun setZeroAltitude() {
        altOffsetDegrees -= snapshot().altDegrees
        persistOffsets()
    }

    fun alignTo(azimuth: Double, altitude: Double) {
        val current = snapshot()
        azOffsetDegrees += Astronomy.angularDifference(azimuth, current.azDegrees)
        altOffsetDegrees += altitude - current.altDegrees
        persistOffsets()
    }

    fun alignToPolaris() = alignTo(0.0, latitude)

    fun beginTwoStarAlignment(expectedAzimuth: Double, expectedAltitude: Double) {
        val current = snapshot()
        preferences.edit()
            .putFloat("star1RawAz", rawAzTicks.toFloat())
            .putFloat("star1RawAlt", rawAltTicks.toFloat())
            .putFloat("star1ExpectedAz", expectedAzimuth.toFloat())
            .putFloat("star1ExpectedAlt", expectedAltitude.toFloat())
            .putBoolean("star1Set", true)
            .putFloat("star1MeasuredAz", current.azDegrees.toFloat())
            .putFloat("star1MeasuredAlt", current.altDegrees.toFloat())
            .apply()
    }

    fun hasFirstAlignmentStar(): Boolean = preferences.getBoolean("star1Set", false)

    fun finishTwoStarAlignment(expectedAzimuth: Double, expectedAltitude: Double): Boolean {
        if (!hasFirstAlignmentStar()) return false
        val measuredAz1 = preferences.getFloat("star1MeasuredAz", 0.0f).toDouble()
        val measuredAlt1 = preferences.getFloat("star1MeasuredAlt", 0.0f).toDouble()
        val expectedAz1 = preferences.getFloat("star1ExpectedAz", 0.0f).toDouble()
        val expectedAlt1 = preferences.getFloat("star1ExpectedAlt", 0.0f).toDouble()
        val current = snapshot()
        val azCorrection1 = Astronomy.angularDifference(expectedAz1, measuredAz1)
        val azCorrection2 = Astronomy.angularDifference(expectedAzimuth, current.azDegrees)
        val sinMean = kotlin.math.sin(azCorrection1 * Math.PI / 180.0) +
            kotlin.math.sin(azCorrection2 * Math.PI / 180.0)
        val cosMean = kotlin.math.cos(azCorrection1 * Math.PI / 180.0) +
            kotlin.math.cos(azCorrection2 * Math.PI / 180.0)
        val averageAzCorrection = Math.toDegrees(kotlin.math.atan2(sinMean, cosMean))
        val averageAltCorrection =
            ((expectedAlt1 - measuredAlt1) + (expectedAltitude - current.altDegrees)) / 2.0
        azOffsetDegrees += averageAzCorrection
        altOffsetDegrees += averageAltCorrection
        preferences.edit().putBoolean("star1Set", false).apply()
        persistOffsets()
        return true
    }

    fun calibrationStart() {
        preferences.edit()
            .putInt("calibrationStartAz", rawAzTicks)
            .putInt("calibrationStartAlt", rawAltTicks)
            .putBoolean("calibrationStarted", true)
            .apply()
        azOffsetDegrees -= snapshot().azDegrees
        altOffsetDegrees -= snapshot().altDegrees
        persistOffsets()
    }

    fun finishAzimuthCalibration(): Double? {
        if (!preferences.getBoolean("calibrationStarted", false)) return null
        val delta = abs(rawAzTicks - preferences.getInt("calibrationStartAz", rawAzTicks)).toDouble()
        if (delta < 1.0) return null
        ticksPerRevAz = delta
        return delta
    }

    fun finishAltitudeCalibration(): Double? {
        if (!preferences.getBoolean("calibrationStarted", false)) return null
        val quarterTurn = abs(rawAltTicks - preferences.getInt("calibrationStartAlt", rawAltTicks)).toDouble()
        if (quarterTurn < 1.0) return null
        ticksPerRevAlt = quarterTurn * 4.0
        return ticksPerRevAlt
    }

    fun calibrationDeltaAz(): Int =
        rawAzTicks - preferences.getInt("calibrationStartAz", rawAzTicks)

    fun calibrationDeltaAlt(): Int =
        rawAltTicks - preferences.getInt("calibrationStartAlt", rawAltTicks)

    fun calibrationStarted(): Boolean = preferences.getBoolean("calibrationStarted", false)

    fun endCalibration() {
        preferences.edit().putBoolean("calibrationStarted", false).apply()
    }

    fun stepSimulation(axis: String, degrees: Double) {
        when (axis) {
            "AZ" -> simulationAzTicks += (degrees / 360.0 * safeResolution(ticksPerRevAz)).roundToInt()
            "ALT" -> simulationAltTicks += (degrees / 360.0 * safeResolution(ticksPerRevAlt)).roundToInt()
            else -> throw IllegalArgumentException("Axe de simulation inconnu.")
        }
    }

    fun resetSimulation() {
        simulationAzTicks = 0
        simulationAltTicks = 0
    }

    fun setConfiguration(
        ticksAz: Double,
        ticksAlt: Double,
        reverseAzimuth: Boolean,
        reverseAltitude: Boolean,
        site: ObservationSite
    ) {
        require(ticksAz.isFinite() && ticksAz > 0.0) { "La résolution AZ doit être positive." }
        require(ticksAlt.isFinite() && ticksAlt > 0.0) { "La résolution ALT doit être positive." }
        require(site.latitude in -90.0..90.0) { "La latitude doit être comprise entre -90° et 90°." }
        require(site.longitude in -180.0..180.0) { "La longitude doit être comprise entre -180° et 180°." }
        ticksPerRevAz = ticksAz
        ticksPerRevAlt = ticksAlt
        reverseAz = reverseAzimuth
        reverseAlt = reverseAltitude
        setSite(site)
    }

    fun allSites(): List<ObservationSite> {
        val stored = preferences.getString("sites", null) ?: return defaults.values.toList()
        return try {
            val array = JSONArray(stored)
            List(array.length()) { index ->
                val site = array.getJSONObject(index)
                ObservationSite(site.getString("name"), site.getDouble("lat"), site.getDouble("lon"))
            }
        } catch (error: Exception) {
            throw IllegalStateException("La liste des lieux enregistrés est illisible.", error)
        }
    }

    fun selectSite(name: String) {
        val site = allSites().firstOrNull { it.name == name }
            ?: throw IllegalArgumentException("Lieu d'observation introuvable.")
        setSite(site)
    }

    fun addSite(site: ObservationSite) {
        require(site.name.isNotBlank()) { "Le nom du lieu est obligatoire." }
        require(site.latitude in -90.0..90.0) { "La latitude doit être comprise entre -90° et 90°." }
        require(site.longitude in -180.0..180.0) { "La longitude doit être comprise entre -180° et 180°." }
        val sites = allSites().filterNot { it.name.equals(site.name, ignoreCase = true) } + site
        persistSites(sites)
        setSite(site)
    }

    fun deleteSite(name: String) {
        val sites = allSites().filterNot { it.name == name }
        require(sites.isNotEmpty()) { "Il faut conserver au moins un lieu d'observation." }
        persistSites(sites)
        if (siteName == name) setSite(sites.first())
    }

    fun setSite(site: ObservationSite) {
        latitude = site.latitude
        longitude = site.longitude
        siteName = site.name
        val sites = allSites().filterNot { it.name == site.name } + site
        persistSites(sites)
    }

    fun updateGpsCoordinates(latitudeDegrees: Double, longitudeDegrees: Double) {
        require(latitudeDegrees in -90.0..90.0) { "La latitude GPS est invalide." }
        require(longitudeDegrees in -180.0..180.0) { "La longitude GPS est invalide." }
        latitude = latitudeDegrees
        longitude = longitudeDegrees
        siteName = "GPS en direct"
    }

    fun selectTarget(target: SkyObject?) {
        activeTarget = target
        selectedTargetName = target?.name ?: ""
    }

    fun restoreTarget(catalog: List<SkyObject>) {
        if (selectedTargetName.isNotBlank()) {
            activeTarget = catalog.firstOrNull { it.name == selectedTargetName }
        }
    }

    fun adjustedTicks(): Pair<Int, Int> {
        val azDirection = if (reverseAz) -1.0 else 1.0
        val altDirection = if (reverseAlt) -1.0 else 1.0
        val azimuthTicks = ((rawAzTicks + simulationAzTicks) * azDirection +
            azOffsetDegrees / 360.0 * safeResolution(ticksPerRevAz)).roundToInt()
        val altitudeTicks = ((rawAltTicks + simulationAltTicks) * altDirection +
            altOffsetDegrees / 360.0 * safeResolution(ticksPerRevAlt)).roundToInt()
        return azimuthTicks to altitudeTicks
    }

    private fun persistOffsets() {
        preferences.edit()
            .putFloat("azOffset", azOffsetDegrees.toFloat())
            .putFloat("altOffset", altOffsetDegrees.toFloat())
            .apply()
    }

    private fun persistSites(sites: List<ObservationSite>) {
        val array = JSONArray()
        sites.forEach { site ->
            array.put(JSONObject().put("name", site.name).put("lat", site.latitude).put("lon", site.longitude))
        }
        preferences.edit().putString("sites", array.toString()).apply()
    }

    private fun safeResolution(value: Double): Double = if (value.isFinite() && value > 0.0) value else 10000.0
}
