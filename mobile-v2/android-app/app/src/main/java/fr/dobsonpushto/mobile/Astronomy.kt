package fr.dobsonpushto.mobile

import android.content.Context
import org.json.JSONArray
import kotlin.math.PI
import kotlin.math.acos
import kotlin.math.asin
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.floor
import kotlin.math.roundToInt
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

data class SkyObject(
    val name: String,
    val commonName: String,
    val type: String,
    val ra: Double,
    val dec: Double,
    val magnitude: Double
)

data class HorizontalCoordinates(val azimuth: Double, val altitude: Double)

data class ObservationSite(val name: String, val latitude: Double, val longitude: Double)

object Astronomy {
    private const val DEG_TO_RAD = PI / 180.0
    private const val RAD_TO_DEG = 180.0 / PI

    fun loadCatalog(context: Context): List<SkyObject> {
        val json = context.assets.open("catalog.json").bufferedReader().use { it.readText() }
        val array = JSONArray(json)
        return List(array.length()) { index ->
            val item = array.getJSONObject(index)
            SkyObject(
                name = item.getString("name"),
                commonName = item.optString("commonName"),
                type = item.getString("type"),
                ra = item.getDouble("ra"),
                dec = item.getDouble("dec"),
                magnitude = item.getDouble("mag")
            )
        } + planetObjects()
    }

    fun precess(raJ2000: Double, decJ2000: Double, epochMillis: Long = System.currentTimeMillis()): Pair<Double, Double> {
        val calendar = java.util.Calendar.getInstance(java.util.TimeZone.getTimeZone("UTC")).apply {
            timeInMillis = epochMillis
        }
        val currentYear = calendar.get(java.util.Calendar.YEAR) +
            calendar.get(java.util.Calendar.MONTH) / 12.0 +
            calendar.get(java.util.Calendar.DAY_OF_YEAR) / 365.25
        val centuries = (currentYear - 2000.0) / 100.0
        val raRad = raJ2000 * 15.0 * DEG_TO_RAD
        val decRad = decJ2000 * DEG_TO_RAD
        val deltaRaSeconds = (307.5 + 133.6 * sin(raRad) * tan(decRad)) * centuries
        val deltaDecArcseconds = 2004.3 * cos(raRad) * centuries
        val ra = positiveModulo(raJ2000 + deltaRaSeconds / 3600.0, 24.0)
        val dec = (decJ2000 + deltaDecArcseconds / 3600.0).coerceIn(-90.0, 90.0)
        return ra to dec
    }

    fun localSiderealTime(longitude: Double, epochMillis: Long = System.currentTimeMillis()): Double {
        val instant = java.time.Instant.ofEpochMilli(epochMillis)
        val utc = instant.atZone(java.time.ZoneOffset.UTC)
        val year = utc.year
        val month = utc.monthValue
        val day = utc.dayOfMonth
        val hour = utc.hour + utc.minute / 60.0 + utc.second / 3600.0
        var y = year
        var m = month
        if (m <= 2) {
            y -= 1
            m += 12
        }
        val a = floor(y / 100.0)
        val b = 2.0 - a + floor(a / 4.0)
        val jd0 = floor(365.25 * (y + 4716)) +
            floor(30.6001 * (m + 1)) + day + b - 1524.5
        val centuries = (jd0 - 2451545.0) / 36525.0
        val gmst0 = 6.697374558 + 2400.051336 * centuries +
            0.000025862 * centuries * centuries
        val gmst = gmst0 + hour * 1.00273790935
        return positiveModulo(gmst + longitude / 15.0, 24.0)
    }

    fun toHorizontal(
        rightAscensionHours: Double,
        declinationDegrees: Double,
        latitudeDegrees: Double,
        longitudeDegrees: Double,
        epochMillis: Long = System.currentTimeMillis()
    ): HorizontalCoordinates {
        val hourAngle = (localSiderealTime(longitudeDegrees, epochMillis) - rightAscensionHours) *
            15.0 * DEG_TO_RAD
        val latitude = latitudeDegrees * DEG_TO_RAD
        val declination = declinationDegrees * DEG_TO_RAD
        val sinAltitude = (sin(declination) * sin(latitude) +
            cos(declination) * cos(latitude) * cos(hourAngle)).coerceIn(-1.0, 1.0)
        val trueAltitude = asin(sinAltitude) * RAD_TO_DEG
        var apparentAltitude = trueAltitude
        if (trueAltitude > -1.0) {
            val refractionArcminutes =
                1.02 / tan((trueAltitude + 10.3 / (trueAltitude + 5.11)) * DEG_TO_RAD)
            apparentAltitude += refractionArcminutes / 60.0
        }
        val denominator = cos(latitude) * cos(asin(sinAltitude))
        val cosAzimuth = if (kotlin.math.abs(denominator) < 1e-12) {
            1.0
        } else {
            ((sin(declination) - sin(latitude) * sinAltitude) / denominator).coerceIn(-1.0, 1.0)
        }
        var azimuth = acos(cosAzimuth) * RAD_TO_DEG
        if (sin(hourAngle) > 0.0) azimuth = 360.0 - azimuth
        return HorizontalCoordinates(positiveModulo(azimuth, 360.0), apparentAltitude)
    }

    fun timeUntilVisible(
        raHours: Double,
        decDegrees: Double,
        latitude: Double,
        longitude: Double,
        epochMillis: Long = System.currentTimeMillis()
    ): String {
        if (toHorizontal(raHours, decDegrees, latitude, longitude, epochMillis).altitude >= 10.0) {
            return "Visible"
        }
        val latitudeRadians = latitude * DEG_TO_RAD
        val declinationRadians = decDegrees * DEG_TO_RAD
        val denominator = cos(latitudeRadians) * cos(declinationRadians)
        if (kotlin.math.abs(denominator) < 1e-12) return "Non visible cette nuit"
        val cosHourAngle = (sin(10.0 * DEG_TO_RAD) -
            sin(latitudeRadians) * sin(declinationRadians)) / denominator
        if (cosHourAngle > 1.0) return "Non visible cette nuit"
        val riseHourAngle = -acos(cosHourAngle.coerceIn(-1.0, 1.0)) * RAD_TO_DEG / 15.0
        val riseSiderealTime = positiveModulo(raHours + riseHourAngle, 24.0)
        val hoursToWait = positiveModulo(
            riseSiderealTime - localSiderealTime(longitude, epochMillis),
            24.0
        )
        if (hoursToWait > 12.0) return "Non visible cette nuit"
        val hours = floor(hoursToWait).toInt()
        val minutes = ((hoursToWait - hours) * 60.0).roundToInt()
        return "Dans ${hours + minutes / 60}h${(minutes % 60).toString().padStart(2, '0')}"
    }

    fun angularDifference(target: Double, current: Double): Double {
        var difference = target - current
        while (difference > 180.0) difference -= 360.0
        while (difference < -180.0) difference += 360.0
        return difference
    }

    fun formatRa(hours: Double): String {
        val totalSeconds = positiveModulo(hours, 24.0) * 3600.0
        val h = floor(totalSeconds / 3600.0).toInt()
        val m = floor((totalSeconds - h * 3600) / 60.0).toInt()
        val s = floor(totalSeconds - h * 3600 - m * 60).toInt()
        return "%02dh %02dm %02ds".format(h, m, s)
    }

    fun formatDec(degrees: Double): String {
        val absolute = kotlin.math.abs(degrees)
        val d = floor(absolute).toInt()
        val totalSeconds = (absolute - d) * 3600.0
        val m = floor(totalSeconds / 60.0).toInt()
        val s = floor(totalSeconds - m * 60).toInt()
        return "%s%02d° %02d' %02d\"".format(if (degrees >= 0.0) "+" else "-", d, m, s)
    }

    fun formatDegMin(degrees: Double): String {
        val normalized = positiveModulo(degrees, 360.0)
        val d = floor(normalized).toInt()
        val m = floor((normalized - d) * 60.0).toInt()
        return "%03d° %02d'".format(d, m)
    }

    fun positiveModulo(value: Double, modulus: Double): Double =
        ((value % modulus) + modulus) % modulus

    private fun planetObjects(epochMillis: Long = System.currentTimeMillis()): List<SkyObject> {
        val days = epochMillis / 86_400_000.0 - 10_957.5
        data class Orbit(
            val name: String, val magnitude: Double, val perihelion: Double, val axis: Double,
            val eccentricity: Double, val meanLongitude: Double, val dailyMotion: Double
        )
        val orbits = listOf(
            Orbit("Vénus", -4.0, 54.88, 0.7233, 0.0067, 50.11, 1.6021),
            Orbit("Mars", -1.5, 286.50, 1.5237, 0.0934, 19.37, 0.5240),
            Orbit("Jupiter", -2.5, 273.87, 5.2026, 0.0485, 20.02, 0.0831),
            Orbit("Saturne", 0.2, 339.39, 9.5549, 0.0555, 317.02, 0.0335)
        )
        return orbits.map { orbit ->
            val meanAnomaly = positiveModulo(orbit.meanLongitude + orbit.dailyMotion * days, 360.0)
            val eccentricAnomaly = meanAnomaly +
                RAD_TO_DEG * orbit.eccentricity * sin(meanAnomaly * DEG_TO_RAD)
            val x = orbit.axis * (cos(eccentricAnomaly * DEG_TO_RAD) - orbit.eccentricity)
            val y = orbit.axis * sqrt(1.0 - orbit.eccentricity * orbit.eccentricity) *
                sin(eccentricAnomaly * DEG_TO_RAD)
            val eclipticLongitude = positiveModulo(
                atan2(y, x) * RAD_TO_DEG + orbit.perihelion, 360.0
            )
            val longitudeRad = eclipticLongitude * DEG_TO_RAD
            SkyObject(
                name = orbit.name,
                commonName = "",
                type = "Planète",
                ra = positiveModulo(eclipticLongitude / 15.0, 24.0),
                dec = 23.44 * sin(longitudeRad),
                magnitude = orbit.magnitude
            )
        }
    }
}
