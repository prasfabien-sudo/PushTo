package fr.dobsonpushto.mobile

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothStatusCodes
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.graphics.BitmapFactory
import android.graphics.drawable.GradientDrawable
import android.hardware.GeomagneticField
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.ParcelUuid
import android.provider.OpenableColumns
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.io.File
import java.util.Locale
import java.util.UUID
import kotlin.math.abs
import kotlin.math.asin
import kotlin.math.atan2
import kotlin.math.sqrt

class MobileMainActivity : Activity() {
    private val handler = Handler(Looper.getMainLooper())
    private val serviceUuid = UUID.fromString("77b7a000-79c4-4baf-9b33-7f6a9d3d0001")
    private val telemetryUuid = UUID.fromString("77b7a001-79c4-4baf-9b33-7f6a9d3d0001")
    private val cccdUuid = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
    private val model by lazy { TelescopeModel(this) }
    private lateinit var objects: List<SkyObject>
    private lateinit var bluetoothAdapter: BluetoothAdapter
    private lateinit var root: LinearLayout
    private lateinit var screenContainer: LinearLayout
    private lateinit var statusLabel: TextView
    private lateinit var navigation: LinearLayout
    private val primaryNavigationButtons = linkedMapOf<String, Button>()
    private val discoveredDevices = linkedMapOf<String, BluetoothDevice>()
    private var scanner: BluetoothLeScanner? = null
    private var scanning = false
    private var gatt: BluetoothGatt? = null
    private var activeScreen = SCREEN_HOME
    private var selectedType = "Tous"
    private var magnitudeLimit = 99.0
    private var visibleOnly = false
    private var skyDome: SkyDomeView? = null
    private var skySensorStatusLabel: TextView? = null
    private var skyCameraPreview: SkyCameraPreview? = null
    private var skyArMode = false
    private var skySensorManager: SensorManager? = null
    private var skySensorListener: SensorEventListener? = null
    private val skyAccelerometer = FloatArray(3)
    private val skyMagneticField = FloatArray(3)
    private var hasSkyAccelerometer = false
    private var hasSkyMagneticField = false
    private var phoneAzimuth = 0.0
    private var phoneAltitude = 45.0
    private var guidanceView: GuidanceView? = null
    private var liveAzLabel: TextView? = null
    private var liveAltLabel: TextView? = null
    private var liveTicksLabel: TextView? = null
    private var liveTargetLabel: TextView? = null
    private var skySafariStatusLabel: TextView? = null
    private var calibrationLabels: List<TextView> = emptyList()
    private var searchQuery = ""
    private var selectedSiteSpinner: Spinner? = null
    private var siteLatitudeField: EditText? = null
    private var siteLongitudeField: EditText? = null
    private var pendingGpsResult: ((Location) -> Unit)? = null
    private var locationListener: LocationListener? = null
    private var gpsTrackingEnabled = false
    private var gpsTrackingListener: LocationListener? = null
    private var gpsStatusLabel: TextView? = null
    private var plateImage: File? = null
    private var plateCaptureFile: File? = null
    private var plateImagePreview: ImageView? = null
    private var plateImageLabel: TextView? = null
    private var plateApiKeyField: EditText? = null
    private var plateApiKeyStatus: TextView? = null
    private var storedAstrometryApiKey: String? = null
    private var plateSolveStatus: TextView? = null
    private var plateSolutionLabel: TextView? = null
    private var plateSolution: Triple<Double, Double, HorizontalCoordinates>? = null
    @Volatile private var onlinePlateSolveRunning = false
    private var onlinePlateSolveThread: Thread? = null
    private val telemetryRefresh = object : Runnable {
        override fun run() {
            refreshLiveData()
            handler.postDelayed(this, TELEMETRY_REFRESH_MS)
        }

    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.statusBarColor = Color.rgb(18, 18, 18)
        window.navigationBarColor = Color.rgb(18, 18, 18)
        val manager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        val adapter = manager.adapter
        if (adapter == null) {
            showError("Cet appareil ne dispose pas du Bluetooth.")
            return
        }
        bluetoothAdapter = adapter
        scanner = adapter.bluetoothLeScanner
        try {
            objects = Astronomy.loadCatalog(this)
            model.restoreTarget(objects)
        } catch (error: Exception) {
            showError("Impossible de charger le catalogue astronomique : ${error.message ?: "erreur de lecture"}.")
            return
        }
        buildAppShell()
        renderScreen(activeScreen)
        handler.post(telemetryRefresh)
    }

    private fun buildAppShell() {
        root = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setBackgroundColor(backgroundColor())
        }
        val header = LinearLayout(this).apply {
            gravity = Gravity.CENTER_VERTICAL
            setPadding(dp(18), dp(14), dp(18), dp(10))
            setBackgroundColor(backgroundColor())
        }
        val brand = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER_VERTICAL
        }
        brand.addView(text("DOBSON PUSH-TO", 18f, accentColor(), true))
        brand.addView(text("Pilotage astronomique", 11f, secondaryText(), false))
        header.addView(brand, LinearLayout.LayoutParams(0, dp(48), 1f))
        root.addView(header)

        statusLabel = text("BLUETOOTH · déconnecté", 12f, secondaryText(), true)
        statusLabel.setPadding(dp(18), dp(8), dp(18), dp(10))
        root.addView(statusLabel)

        val scroll = ScrollView(this)
        screenContainer = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(16), dp(4), dp(16), dp(18))
        }
        scroll.addView(screenContainer)
        root.addView(scroll, LinearLayout.LayoutParams(-1, 0, 1f))

        navigation = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(dp(8), dp(7), dp(8), dp(7))
            setBackgroundColor(if (model.nightMode) Color.rgb(38, 0, 0) else Color.rgb(20, 29, 42))
        }
        primaryNavigationButtons.clear()
        PRIMARY_NAVIGATION.forEach { (key, label) ->
            val isNightShortcut = key == SCREEN_NIGHT
            val item = button(
                if (isNightShortcut) {
                    if (model.nightMode) "Jour" else "Nuit"
                } else label
            ).apply {
                textSize = 11f
                setPadding(dp(3), dp(4), dp(3), dp(4))
                if (isNightShortcut) {
                    contentDescription =
                        if (model.nightMode) "Désactiver le mode nuit" else "Activer le mode nuit"
                    setTextColor(if (model.nightMode) Color.rgb(38, 0, 0) else Color.WHITE)
                    background = roundedBackground(
                        if (model.nightMode) Color.WHITE else Color.rgb(220, 30, 50),
                        dp(10),
                        if (model.nightMode) Color.WHITE else Color.rgb(220, 30, 50)
                    )
                }
                setOnClickListener {
                    when (key) {
                        SCREEN_MORE -> showMoreNavigation()
                        SCREEN_NIGHT -> {
                            model.nightMode = !model.nightMode
                            rebuildShell()
                        }
                        else -> renderScreen(key)
                    }
                }
            }
            if (key != SCREEN_MORE && !isNightShortcut) primaryNavigationButtons[key] = item
            navigation.addView(item, LinearLayout.LayoutParams(0, dp(48), 1f).apply {
                marginEnd = dp(4)
            })
        }
        root.addView(navigation)
        setContentView(root)
    }

    private fun rebuildShell() {
        buildAppShell()
        renderScreen(activeScreen)
    }

    private fun renderScreen(screen: String) {
        if (activeScreen == SCREEN_SKY_3D) {
            stopSkySensors()
            stopSkyCameraPreview()
        }
        if (activeScreen == SCREEN_SKY_3D && screen != SCREEN_SKY_3D && gpsTrackingEnabled) {
            stopGpsTracking()
        }
        activeScreen = screen
        screenContainer.removeAllViews()
        liveAzLabel = null
        liveAltLabel = null
        liveTicksLabel = null
        liveTargetLabel = null
        guidanceView = null
        skySafariStatusLabel = null
        skyDome = null
        skySensorStatusLabel = null
        skyCameraPreview = null
        gpsStatusLabel = null
        plateImagePreview = null
        plateImageLabel = null
        plateApiKeyField = null
        plateApiKeyStatus = null
        storedAstrometryApiKey = null
        plateSolveStatus = null
        plateSolutionLabel = null
        calibrationLabels = emptyList()
        selectedSiteSpinner = null
        siteLatitudeField = null
        siteLongitudeField = null
        primaryNavigationButtons.forEach { (key, item) ->
            val selected = key == screen
            item.alpha = if (selected) 1f else 0.78f
            item.setTextColor(if (selected) accentColor() else secondaryText())
            item.background = roundedBackground(
                if (selected) cardColor() else backgroundColor(),
                dp(10),
                if (selected) borderColor() else backgroundColor()
            )
        }
        when (screen) {
            SCREEN_HOME -> showHome()
            SCREEN_CATALOG -> showCatalog()
            SCREEN_STATION -> showStation()
            SCREEN_SKY_3D -> showSkyDome()
            SCREEN_CONFIG -> showConfiguration()
            SCREEN_CALIBRATION -> showCalibration()
            SCREEN_SIMULATION -> showSimulation()
            SCREEN_SKYSAFARI -> showSkySafari()
            SCREEN_PLATE_SOLVER -> showPlateSolver()
            SCREEN_HELP -> showHelp()
            SCREEN_TESTS -> showTests()
            SCREEN_RELEASES -> showReleaseNotes()
        }
        refreshLiveData()
    }

    private fun showMoreNavigation() {
        val primary = PRIMARY_NAVIGATION.map { it.first }.toSet()
        val choices = SCREEN_LABELS.filterKeys { it !in primary }
        AlertDialog.Builder(this)
            .setTitle("Autres fonctions")
            .setItems(choices.values.toTypedArray()) { _, index ->
                choices.keys.elementAtOrNull(index)?.let(::renderScreen)
            }
            .show()
    }

    private fun showHome() {
        section("Position du télescope")
        liveAzLabel = value("Azimut : --")
        liveAltLabel = value("Altitude : --")
        liveTicksLabel = text("Ticks AZ : --   ·   Ticks ALT : --", 14f, secondaryText(), false)
        val connection = text("Bluetooth : déconnecté", 14f, secondaryText(), false)
        screenContainer.addView(liveAzLabel)
        screenContainer.addView(liveAltLabel)
        screenContainer.addView(liveTicksLabel)
        screenContainer.addView(connection)
        connectButton(connection)

        section("Lieu d'observation")
        screenContainer.addView(value("${model.siteName} · %.4f°, %.4f°".format(model.latitude, model.longitude)))
        screenContainer.addView(button("Choisir un autre lieu").apply {
            setOnClickListener { renderScreen(SCREEN_CONFIG) }
        })

        section("Cible actuelle")
        liveTargetLabel = value("Aucune cible sélectionnée")
        screenContainer.addView(liveTargetLabel)
        val guide = value("Écart AZ/ALT : --")
        screenContainer.addView(guide)
        guidanceView = GuidanceView(this).apply {
            layoutParams = LinearLayout.LayoutParams(-1, dp(185)).apply { bottomMargin = dp(8) }
            nightMode = model.nightMode
        }
        screenContainer.addView(guidanceView)
        screenContainer.addView(button("Parcourir le catalogue").apply {
            setOnClickListener { renderScreen(SCREEN_CATALOG) }
        })
        screenContainer.addView(button("Aligner sur la cible").apply {
            setOnClickListener { alignToSelectedTarget() }
        })
        screenContainer.addView(button("Mise en station").apply {
            setOnClickListener { renderScreen(SCREEN_STATION) }
        })
        screenContainer.addView(button("Vue 3D · capteurs et GPS").apply {
            setOnClickListener { renderScreen(SCREEN_SKY_3D) }
        })
        screenContainer.addView(button("Analyser une image · Astrometry.net").apply {
            setOnClickListener { renderScreen(SCREEN_PLATE_SOLVER) }
        })
    }

    private fun connectButton(label: TextView) {
        screenContainer.addView(button("Rechercher l'ESP32 en Bluetooth").apply {
            setOnClickListener {
                ensurePermissionsThenScan {
                    label.text = "Recherche BLE en cours…"
                }
            }
        })
        screenContainer.addView(button("Déconnecter le télescope").apply {
            setOnClickListener { disconnect() }
        })
        screenContainer.addView(LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            tag = DEVICE_LIST_TAG
        })
    }

    private fun showCatalog() {
        section("Catalogue Messier, NGC et planètes")
        screenContainer.addView(text("${objects.size} objets · triés par proximité du pointage", 14f, secondaryText(), false))
        val search = EditText(this).apply {
            hint = "Rechercher nom, type ou désignation"
            setText(searchQuery)
            setSingleLine(true)
            setTextColor(primaryText())
            setHintTextColor(secondaryText())
            setPadding(dp(12), dp(10), dp(12), dp(10))
        }
        screenContainer.addView(search, matchWidth())
        val typeSpinner = spinner(listOf("Tous", "Planètes", "Galaxies", "Nébuleuses", "Amas", "Étoiles"), selectedType)
        typeSpinner.onItemSelectedListener = simpleSelection { selectedType = it; renderCatalogRows() }
        screenContainer.addView(typeSpinner)
        val magnitudeSpinner = spinner(
            listOf("Toutes magnitudes", "≤ 6 · œil nu", "≤ 8 · jumelles", "≤ 10 · petit télescope", "≤ 12 · grand télescope"),
            magnitudeLabel(magnitudeLimit)
        )
        magnitudeSpinner.onItemSelectedListener = simpleSelection {
            magnitudeLimit = when (it) {
                "≤ 6 · œil nu" -> 6.0
                "≤ 8 · jumelles" -> 8.0
                "≤ 10 · petit télescope" -> 10.0
                "≤ 12 · grand télescope" -> 12.0
                else -> 99.0
            }
            renderCatalogRows()
        }
        screenContainer.addView(magnitudeSpinner)
        val visibleCheck = CheckBox(this).apply {
            text = "Afficher seulement les objets au-dessus de 10°"
            isChecked = visibleOnly
            setTextColor(primaryText())
            setOnCheckedChangeListener { _, checked -> visibleOnly = checked; renderCatalogRows() }
        }
        screenContainer.addView(visibleCheck)
        val rows = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL; tag = CATALOG_ROWS_TAG }
        screenContainer.addView(rows)
        search.addTextChangedListener(simpleTextChange { searchQuery = it; renderCatalogRows() })
        renderCatalogRows()
    }

    private fun renderCatalogRows() {
        val rows = screenContainer.findViewWithTag<LinearLayout>(CATALOG_ROWS_TAG) ?: return
        rows.removeAllViews()
        val now = System.currentTimeMillis()
        val current = model.snapshot()
        val matches = objects.map { obj ->
            val (ra, dec) = Astronomy.precess(obj.ra, obj.dec, now)
            val horizontal = Astronomy.toHorizontal(ra, dec, model.latitude, model.longitude, now)
            val deltaAz = Astronomy.angularDifference(horizontal.azimuth, current.azDegrees)
            val deltaAlt = horizontal.altitude - current.altDegrees
            Triple(obj, horizontal, sqrt(deltaAz * deltaAz + deltaAlt * deltaAlt))
        }.filter { (obj, horizontal, _) ->
            val query = searchQuery.trim().lowercase()
            val type = obj.type.lowercase()
            val typeMatch = when (selectedType) {
                "Planètes" -> type.contains("planète") || type.contains("planete")
                "Galaxies" -> type.contains("gal")
                "Nébuleuses" -> type.contains("neb")
                "Amas" -> type.contains("amas")
                "Étoiles" -> !type.contains("gal") && !type.contains("neb") && !type.contains("amas") && !type.contains("plan")
                else -> true
            }
            typeMatch && obj.magnitude <= magnitudeLimit &&
                (!visibleOnly || horizontal.altitude >= 10.0) &&
                (query.isEmpty() || obj.name.lowercase().contains(query) ||
                    obj.commonName.lowercase().contains(query) || type.contains(query))
        }.sortedBy { it.third }
        if (matches.isEmpty()) {
            rows.addView(text("Aucun objet ne correspond aux filtres.", 15f, secondaryText(), false))
            return
        }
        matches.forEach { (obj, coordinates, distance) ->
            val card = card()
            val title = obj.commonName.ifBlank { obj.name }
            card.addView(text("$title · ${obj.name}", 16f, primaryText(), true))
            card.addView(text(
                "${obj.type} · mag %.1f · ${if (coordinates.altitude >= 10) "Visible" else "Sous 10°"}".format(obj.magnitude),
                13f,
                secondaryText(),
                false
            ))
            card.addView(text(
                Astronomy.timeUntilVisible(obj.ra, obj.dec, model.latitude, model.longitude, now),
                13f,
                secondaryText(),
                false
            ))
            card.addView(text(
                "RA ${Astronomy.formatRa(Astronomy.precess(obj.ra, obj.dec).first)} · DEC ${Astronomy.formatDec(Astronomy.precess(obj.ra, obj.dec).second)}",
                13f,
                secondaryText(),
                false
            ))
            card.addView(text(
                "AZ ${Astronomy.formatDegMin(coordinates.azimuth)} · ALT %.1f° · écart %.1f°".format(coordinates.altitude, distance),
                13f,
                accentColor(),
                false
            ))
            card.addView(button(if (model.activeTarget?.name == obj.name) "Cible sélectionnée" else "Choisir comme cible").apply {
                setOnClickListener {
                    model.selectTarget(obj)
                    renderScreen(SCREEN_HOME)
                }
            })
            rows.addView(card)
        }
    }

    private fun showStation() {
        section("Calage rapide")
        screenContainer.addView(button("Mettre l'altitude à zéro · tube horizontal").apply {
            setOnClickListener { model.setZeroAltitude(); refreshLiveData(); toast("Altitude recalée à 0°") }
        })
        screenContainer.addView(button("Mettre l'azimut à zéro · pointer le Nord").apply {
            setOnClickListener { model.setZeroAzimuth(); refreshLiveData(); toast("Azimut recalé à 0°") }
        })
        screenContainer.addView(button("Recaler sur Polaris · AZ 0°, ALT latitude").apply {
            setOnClickListener { model.alignToPolaris(); refreshLiveData(); toast("Pointage recalé sur Polaris") }
        })
        section("Alignement sur une étoile")
        val starAz = input("Azimut théorique (°)", "120")
        val starAlt = input("Altitude théorique (°)", "45")
        screenContainer.addView(starAz)
        screenContainer.addView(starAlt)
        screenContainer.addView(button("Aligner sur cette position").apply {
            setOnClickListener {
                val az = starAz.number()
                val alt = starAlt.number()
                if (az == null || alt == null || !validateHorizontal(az, alt)) return@setOnClickListener
                model.alignTo(az, alt)
                refreshLiveData()
                toast("Pointage recalé sur l'étoile")
            }
        })
        section("Alignement guidé sur deux étoiles")
        screenContainer.addView(text(
            if (model.hasFirstAlignmentStar()) {
                "Étoile 1 enregistrée. Pointez la seconde étoile, puis saisissez ses coordonnées."
            } else {
                "Pointez la première étoile et saisissez ses coordonnées célestes AZ/ALT."
            },
            14f,
            secondaryText(),
            false
        ))
        val pairAz = input("Azimut théorique (°)", "")
        val pairAlt = input("Altitude théorique (°)", "")
        screenContainer.addView(pairAz)
        screenContainer.addView(pairAlt)
        screenContainer.addView(button(if (model.hasFirstAlignmentStar()) "Valider étoile 2 et terminer" else "Enregistrer étoile 1").apply {
            setOnClickListener {
                val az = pairAz.number()
                val alt = pairAlt.number()
                if (az == null || alt == null || !validateHorizontal(az, alt)) return@setOnClickListener
                if (model.hasFirstAlignmentStar()) {
                    if (!model.finishTwoStarAlignment(az, alt)) {
                        toast("Aucune première étoile n'est enregistrée.")
                    } else {
                        toast("Alignement deux étoiles terminé.")
                    }
                } else {
                    model.beginTwoStarAlignment(az, alt)
                    toast("Première étoile enregistrée. Pointez la seconde.")
                }
                renderScreen(SCREEN_STATION)
            }
        })
    }

    private fun showSkyDome() {
        section(if (skyArMode) "Réalité augmentée · ciel" else "Vue céleste immersive")
        val current = model.snapshot()
        val dome = SkyDomeView(this).apply {
            layoutParams = FrameLayout.LayoutParams(-1, -1)
            objects = this@MobileMainActivity.objects
            latitude = model.latitude
            longitude = model.longitude
            azimuth = current.azDegrees
            altitude = current.altDegrees
            phoneAzimuth = this@MobileMainActivity.phoneAzimuth
            phoneAltitude = this@MobileMainActivity.phoneAltitude
            phoneFieldView = true
            phoneSensorActive = false
            cameraOverlay = skyArMode
            nightMode = model.nightMode
            sensorConnected = model.connected
            lastTelemetryAt = model.lastTelemetryAt
            selectedObject = model.activeTarget
            onObjectSelected = {
                model.selectTarget(it)
                refreshLiveData()
            }
        }
        skyDome = dome
        val scene = FrameLayout(this).apply {
            layoutParams = LinearLayout.LayoutParams(-1, dp(430)).apply { bottomMargin = dp(8) }
            setBackgroundColor(if (model.nightMode) Color.rgb(18, 2, 7) else Color.rgb(4, 9, 20))
        }
        if (skyArMode) {
            skyCameraPreview = SkyCameraPreview(this) { message ->
                runOnUiThread {
                    skySensorStatusLabel?.text = message
                    if (!isFinishing && !isDestroyed) toast(message)
                }
            }.also { preview ->
                scene.addView(preview, FrameLayout.LayoutParams(-1, -1))
            }
        }
        scene.addView(dome, FrameLayout.LayoutParams(-1, -1))
        screenContainer.addView(scene)

        skySensorStatusLabel = value("Initialisation des capteurs d'orientation…")
        screenContainer.addView(skySensorStatusLabel)
        screenContainer.addView(button(if (skyArMode) "Vue céleste sans caméra" else "Réalité augmentée avec caméra").apply {
            setOnClickListener {
                if (skyArMode) {
                    skyArMode = false
                    renderScreen(SCREEN_SKY_3D)
                } else {
                    requestMissing(arrayOf(Manifest.permission.CAMERA), REQUEST_CAMERA_PERMISSION) {
                        skyArMode = true
                        renderScreen(SCREEN_SKY_3D)
                    }
                }
            }
        })
        screenContainer.addView(text(
            "Pointe l'arrière du téléphone vers le ciel : la boussole et l'inclinomètre placent les objets dans le champ. Touche un objet pour le sélectionner.",
            13f,
            secondaryText(),
            false
        ))

        gpsStatusLabel = value("Position d'observation : ${model.siteName} · %.4f°, %.4f°".format(model.latitude, model.longitude))
        screenContainer.addView(gpsStatusLabel)
        screenContainer.addView(button(if (gpsTrackingEnabled) "Arrêter le suivi GPS" else "Utiliser le GPS en direct").apply {
            setOnClickListener {
                if (gpsTrackingEnabled) stopGpsTracking()
                else ensureLocationPermissionThenTrack()
            }
        })
        screenContainer.addView(text(
            "La visée suit les capteurs internes du téléphone ; le réticule vert indique séparément le pointage des encodeurs ESP32. Le GPS actualise le lieu d'observation.",
            13f,
            secondaryText(),
            false
        ))
        screenContainer.addView(button("Plate solving · analyser une image").apply {
            setOnClickListener { renderScreen(SCREEN_PLATE_SOLVER) }
        })
        screenContainer.addView(button("Choisir une cible").apply {
            setOnClickListener { renderScreen(SCREEN_CATALOG) }
        })
        startSkySensors()
        if (skyArMode) {
            requestMissing(arrayOf(Manifest.permission.CAMERA), REQUEST_CAMERA_PERMISSION) {
                skyCameraPreview?.start()
            }
        }
    }

    private fun startSkySensors() {
        if (skySensorListener != null) return
        val manager = getSystemService(Context.SENSOR_SERVICE) as SensorManager
        skySensorManager = manager
        val rotationVector = manager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
        val accelerometer = manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
        val magneticSensor = manager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)
        val listener = object : SensorEventListener {
            override fun onSensorChanged(event: SensorEvent) {
                when (event.sensor.type) {
                    Sensor.TYPE_ROTATION_VECTOR -> {
                        val matrix = FloatArray(9)
                        SensorManager.getRotationMatrixFromVector(matrix, event.values)
                        updatePhoneDirection(matrix)
                    }
                    Sensor.TYPE_ACCELEROMETER -> {
                        event.values.copyInto(skyAccelerometer)
                        hasSkyAccelerometer = true
                        updatePhoneDirectionFromCompass()
                    }
                    Sensor.TYPE_MAGNETIC_FIELD -> {
                        event.values.copyInto(skyMagneticField)
                        hasSkyMagneticField = true
                        updatePhoneDirectionFromCompass()
                    }
                }
            }

            override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {
                skySensorStatusLabel?.text = when (accuracy) {
                    SensorManager.SENSOR_STATUS_UNRELIABLE -> "Boussole imprécise · éloigne le téléphone des aimants."
                    SensorManager.SENSOR_STATUS_ACCURACY_LOW -> "Boussole active · précision faible."
                    else -> "Capteurs actifs · pointe le dos du téléphone vers le ciel."
                }
            }
        }
        skySensorListener = listener
        if (rotationVector != null) {
            manager.registerListener(listener, rotationVector, SensorManager.SENSOR_DELAY_GAME)
        } else if (accelerometer != null && magneticSensor != null) {
            hasSkyAccelerometer = false
            hasSkyMagneticField = false
            manager.registerListener(listener, accelerometer, SensorManager.SENSOR_DELAY_GAME)
            manager.registerListener(listener, magneticSensor, SensorManager.SENSOR_DELAY_GAME)
        } else {
            skySensorStatusLabel?.text = "Ce téléphone ne fournit pas les capteurs nécessaires à l'orientation céleste."
            return
        }
        skyDome?.phoneSensorActive = true
        skySensorStatusLabel?.text = "Capteurs actifs · pointe le dos du téléphone vers le ciel."
    }

    private fun updatePhoneDirectionFromCompass() {
        if (!hasSkyAccelerometer || !hasSkyMagneticField) return
        val matrix = FloatArray(9)
        val inclination = FloatArray(9)
        if (SensorManager.getRotationMatrix(matrix, inclination, skyAccelerometer, skyMagneticField)) {
            updatePhoneDirection(matrix)
        }
    }

    private fun updatePhoneDirection(matrix: FloatArray) {
        val east = -matrix[2].toDouble()
        val north = -matrix[5].toDouble()
        val up = -matrix[8].toDouble()
        val declination = GeomagneticField(
            model.latitude.toFloat(),
            model.longitude.toFloat(),
            0f,
            System.currentTimeMillis()
        ).declination.toDouble()
        phoneAzimuth = Astronomy.positiveModulo(Math.toDegrees(atan2(east, north)) + declination, 360.0)
        phoneAltitude = Math.toDegrees(asin(up.coerceIn(-1.0, 1.0)))
        skyDome?.apply {
            phoneAzimuth = this@MobileMainActivity.phoneAzimuth
            phoneAltitude = this@MobileMainActivity.phoneAltitude
            phoneSensorActive = true
        }
        skySensorStatusLabel?.text =
            "Boussole %.0f° · altitude visée %.0f°".format(phoneAzimuth, phoneAltitude)
    }

    private fun stopSkySensors() {
        val listener = skySensorListener ?: return
        skySensorManager?.unregisterListener(listener)
        skySensorListener = null
        skySensorManager = null
        skyDome?.phoneSensorActive = false
    }

    private fun stopSkyCameraPreview() {
        skyCameraPreview?.stop()
        skyCameraPreview = null
    }

    private fun ensureLocationPermissionThenTrack() {
        requestMissing(arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), REQUEST_LOCATION_PERMISSION) {
            startGpsTracking()
        }
    }

    private fun startGpsTracking() {
        if (gpsTrackingListener != null) return
        val locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager
        val providers = listOf(LocationManager.GPS_PROVIDER, LocationManager.NETWORK_PROVIDER)
            .filter { provider ->
                try {
                    locationManager.isProviderEnabled(provider)
                } catch (error: IllegalArgumentException) {
                    false
                }
            }
        if (providers.isEmpty()) {
            toast("Active le GPS ou la localisation réseau du téléphone, puis réessaie.")
            return
        }

        gpsTrackingEnabled = true
        val listener = object : LocationListener {
            override fun onLocationChanged(location: Location) {
                model.updateGpsCoordinates(location.latitude, location.longitude)
                val accuracy = if (location.hasAccuracy()) " · précision ±%.0f m".format(location.accuracy) else ""
                gpsStatusLabel?.text =
                    "GPS en direct · %.5f°, %.5f°$accuracy".format(location.latitude, location.longitude)
                refreshLiveData()
            }
        }
        gpsTrackingListener = listener
        try {
            providers.forEach { provider ->
                locationManager.requestLocationUpdates(provider, 5_000L, 10f, listener, Looper.getMainLooper())
            }
            gpsStatusLabel?.text = "GPS en cours d'acquisition…"
        } catch (error: SecurityException) {
            gpsTrackingEnabled = false
            gpsTrackingListener = null
            toast("Permission de localisation manquante : ${error.message ?: "accès refusé"}.")
        } catch (error: IllegalArgumentException) {
            gpsTrackingEnabled = false
            gpsTrackingListener = null
            toast("Impossible de démarrer le GPS : ${error.message ?: "fournisseur indisponible"}.")
        }
    }

    private fun stopGpsTracking(clearRequested: Boolean = true) {
        val listener = gpsTrackingListener
        if (listener != null) {
            try {
                (getSystemService(Context.LOCATION_SERVICE) as LocationManager).removeUpdates(listener)
            } catch (error: SecurityException) {
                toast("Impossible d'arrêter le suivi GPS : ${error.message ?: "permission manquante"}.")
            }
        }
        gpsTrackingListener = null
        if (clearRequested) gpsTrackingEnabled = false
        if (clearRequested) gpsStatusLabel?.text = "GPS arrêté · ${model.siteName}"
    }

    private fun showPlateSolver() {
        section("Plate solving")
        screenContainer.addView(text(
            "Choisis une image astronomique (ou prends-en une avec l'appareil photo) pour la faire analyser en ligne par Astrometry.net.",
            14f,
            secondaryText(),
            false
        ))

        plateImagePreview = ImageView(this).apply {
            layoutParams = LinearLayout.LayoutParams(-1, dp(190)).apply { bottomMargin = dp(6) }
            scaleType = ImageView.ScaleType.FIT_CENTER
            setBackground(roundedBackground(cardColor(), dp(14), borderColor()))
            contentDescription = "Aperçu de l'image choisie pour le plate solving"
        }
        screenContainer.addView(plateImagePreview)
        plateImageLabel = value("Aucune image sélectionnée")
        screenContainer.addView(plateImageLabel)
        screenContainer.addView(button("Prendre une photo").apply { setOnClickListener { capturePlateImage() } })
        screenContainer.addView(button("Choisir une image").apply { setOnClickListener { choosePlateImage() } })

        section("Astrometry.net")
        plateSolveStatus = value("Choisis une image pour la résoudre avec Astrometry.net.")
        screenContainer.addView(plateSolveStatus)
        screenContainer.addView(text(
            "La photo sera envoyée à nova.astrometry.net après confirmation. La clé peut être enregistrée chiffrée sur cet appareil.",
            13f,
            secondaryText(),
            false
        ))
        screenContainer.addView(button("Comment obtenir une clé API ?").apply {
            setOnClickListener {
                AlertDialog.Builder(this@MobileMainActivity)
                    .setTitle("Clé API Astrometry.net")
                    .setMessage("Crée un compte sur nova.astrometry.net, puis récupère ta clé API dans les paramètres de ton compte. Elle est enregistrée uniquement sur cet appareil, chiffrée avec Android Keystore, et transmise au service pour l'authentification.")
                    .setPositiveButton("Compris", null)
                    .show()
            }
        })
        var keyLoadError: String? = null
        val storedApiKey = try {
            AstrometryApiKeyStore(this).load()
        } catch (error: Exception) {
            keyLoadError = "Clé enregistrée inaccessible : ${error.message ?: "erreur de chiffrement"}."
            null
        }
        storedAstrometryApiKey = storedApiKey
        plateApiKeyField = input("Clé API Astrometry.net", storedApiKey.orEmpty()).apply {
            inputType = android.text.InputType.TYPE_CLASS_TEXT or
                android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD
            setSaveEnabled(false)
            addTextChangedListener(simpleTextChange { updatePlateSolverControls() })
        }
        screenContainer.addView(plateApiKeyField)
        plateApiKeyStatus = value(
            keyLoadError ?: if (storedApiKey != null) "Clé chiffrée enregistrée sur cet appareil."
            else "Clé non enregistrée."
        )
        screenContainer.addView(plateApiKeyStatus)
        screenContainer.addView(button("Enregistrer la clé sur cet appareil").apply {
            setOnClickListener { saveAstrometryApiKey() }
        })
        screenContainer.addView(button("Supprimer la clé enregistrée").apply {
            isEnabled = storedApiKey != null
            tag = PLATE_API_KEY_CLEAR_BUTTON_TAG
            setOnClickListener { clearAstrometryApiKey() }
        })
        screenContainer.addView(button("Résoudre avec Astrometry.net").apply {
            isEnabled = false
            tag = PLATE_ONLINE_BUTTON_TAG
            setOnClickListener { confirmOnlinePlateSolve() }
        })

        plateSolutionLabel = value("Aucune solution astrométrique")
        screenContainer.addView(plateSolutionLabel)
        screenContainer.addView(button("Aligner le télescope sur cette solution").apply {
            tag = PLATE_ALIGN_BUTTON_TAG
            isEnabled = plateSolution != null
            setOnClickListener {
                val solved = plateSolution ?: return@setOnClickListener
                model.alignTo(solved.third.azimuth, solved.third.altitude)
                toast("Pointage recalé sur la position résolue.")
                refreshLiveData()
            }
        })
        plateImage?.let { displayPlateImage(it) }
        updatePlateSolverControls()
        plateSolution?.let { showPlateSolution(it.first, it.second) }
    }

    @Deprecated("Use Activity Result APIs when the app adopts AndroidX Activity.")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode != RESULT_OK) {
            if (requestCode == REQUEST_CAPTURE_PLATE_IMAGE) {
                plateCaptureFile?.delete()
                plateCaptureFile = null
            }
            return
        }
        when (requestCode) {
            REQUEST_CAPTURE_PLATE_IMAGE -> {
                val image = plateCaptureFile
                plateCaptureFile = null
                if (image == null || !image.isFile || image.length() == 0L) {
                    plateSolveStatus?.text = "La caméra n'a pas fourni d'image exploitable."
                    return
                }
                displayPlateImage(image)
            }
            REQUEST_PICK_PLATE_IMAGE -> {
                val uri = data?.data ?: return
                try {
                    val displayName = contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null)
                        ?.use { cursor ->
                            if (cursor.moveToFirst()) cursor.getString(0) else null
                        }
                        ?.takeIf { it.isNotBlank() }
                        ?: "astro-${System.currentTimeMillis()}.jpg"
                    val suffix = displayName.substringAfterLast('.', "jpg")
                        .lowercase(Locale.ROOT)
                        .filter { it.isLetterOrDigit() }
                        .take(8)
                        .ifBlank { "jpg" }
                    val destination = File(cacheDir, "plate-input-${UUID.randomUUID()}.$suffix")
                    val maxImageBytes = 150L * 1024L * 1024L
                    contentResolver.openInputStream(uri)?.use { input ->
                        destination.outputStream().buffered().use { output ->
                            val buffer = ByteArray(64 * 1024)
                            var total = 0L
                            var count: Int
                            while (input.read(buffer).also { count = it } >= 0) {
                                if (count == 0) continue
                                total += count
                                if (total > maxImageBytes) {
                                    throw IllegalArgumentException("L'image dépasse la limite de 150 Mo.")
                                }
                                output.write(buffer, 0, count)
                            }
                        }
                    } ?: throw IllegalStateException("Impossible d'ouvrir l'image sélectionnée.")
                    displayPlateImage(destination)
                } catch (error: Exception) {
                    plateSolveStatus?.text =
                        "Import impossible : ${error.message ?: "image inaccessible"}."
                }
            }
        }
    }

    private fun choosePlateImage() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "image/*"
        }
        startActivityForResult(intent, REQUEST_PICK_PLATE_IMAGE)
    }

    private fun capturePlateImage() {
        val captureDirectory = File(cacheDir, "plate-capture").apply { mkdirs() }
        val capture = File(captureDirectory, "dobson-${System.currentTimeMillis()}.jpg")
        val uri = androidx.core.content.FileProvider.getUriForFile(
            this,
            "$packageName.fileprovider",
            capture
        )
        val intent = Intent(android.provider.MediaStore.ACTION_IMAGE_CAPTURE).apply {
            putExtra(android.provider.MediaStore.EXTRA_OUTPUT, uri)
            addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION or Intent.FLAG_GRANT_READ_URI_PERMISSION)
        }
        plateCaptureFile = capture
        try {
            startActivityForResult(intent, REQUEST_CAPTURE_PLATE_IMAGE)
        } catch (_: ActivityNotFoundException) {
            plateCaptureFile = null
            capture.delete()
            toast("Impossible d'ouvrir l'appareil photo : aucune application compatible n'est installée.")
        }
    }

    private fun showPlateSolution(raDegrees: Double, decDegrees: Double) {
        val horizontal = Astronomy.toHorizontal(
            raDegrees / 15.0,
            decDegrees,
            model.latitude,
            model.longitude
        )
        plateSolution = Triple(raDegrees, decDegrees, horizontal)
        plateSolveStatus?.text = "Solution trouvée par Astrometry.net · photo envoyée au service."
        plateSolutionLabel?.text =
            "Centre de l'image\nRA ${Astronomy.formatRa(raDegrees / 15.0)} · DEC ${Astronomy.formatDec(decDegrees)}\n" +
                "AZ ${Astronomy.formatDegMin(horizontal.azimuth)} · ALT %.1f°".format(horizontal.altitude)
        screenContainer.findViewWithTag<Button>(PLATE_ALIGN_BUTTON_TAG)?.isEnabled = true
        updatePlateSolverControls()
    }

    private fun updatePlateSolverControls() {
        screenContainer.findViewWithTag<Button>(PLATE_ONLINE_BUTTON_TAG)?.isEnabled =
            plateImage != null &&
                plateApiKeyField?.text?.toString()?.trim() == storedAstrometryApiKey &&
                !storedAstrometryApiKey.isNullOrBlank() &&
                !onlinePlateSolveRunning
    }

    private fun confirmOnlinePlateSolve() {
        val image = plateImage ?: run {
            toast("Choisis d'abord une image.")
            return
        }
        val apiKey = plateApiKeyField?.text?.toString()?.trim().orEmpty()
        if (apiKey.isBlank()) {
            plateSolveStatus?.text = "Saisis une clé API Astrometry.net."
            return
        }
        val storedApiKey = try {
            AstrometryApiKeyStore(this).load()
        } catch (error: Exception) {
            plateApiKeyStatus?.text =
                "Clé enregistrée inaccessible : ${error.message ?: "erreur de chiffrement"}."
            return
        }
        if (storedApiKey != apiKey) {
            plateSolveStatus?.text = "Enregistre d'abord la clé sur cet appareil avant de lancer le solveur en ligne."
            return
        }
        val imageSize = formatBytes(image.length())
        AlertDialog.Builder(this)
            .setTitle("Envoyer la photo à Astrometry.net ?")
            .setMessage(
                "La photo « ${image.name} » ($imageSize) et ta clé API seront transmises à nova.astrometry.net pour calculer la solution. " +
                    "L'application demande que la soumission ne soit pas publiquement visible ; le traitement dépend du service et de ses règles de conservation. " +
                    "Continuer ?"
            )
            .setNegativeButton("Annuler", null)
            .setPositiveButton("Envoyer et résoudre") { _, _ -> startOnlinePlateSolve(image, apiKey) }
            .show()
    }

    private fun startOnlinePlateSolve(image: File, apiKey: String) {
        if (onlinePlateSolveRunning) return
        onlinePlateSolveRunning = true
        plateSolveStatus?.text = "Préparation de l'envoi à Astrometry.net…"
        updatePlateSolverControls()
        val worker = Thread({
            try {
                val solution = AstrometryOnlineClient().solve(image, apiKey) { progress ->
                    runOnUiThread {
                        if (!isFinishing && !isDestroyed) plateSolveStatus?.text = progress
                    }
                }
                runOnUiThread {
                    onlinePlateSolveRunning = false
                    onlinePlateSolveThread = null
                    if (!isFinishing && !isDestroyed) {
                        showPlateSolution(solution.rightAscensionDegrees, solution.declinationDegrees)
                    }
                }
            } catch (error: Exception) {
                runOnUiThread {
                    onlinePlateSolveRunning = false
                    onlinePlateSolveThread = null
                    if (!isFinishing && !isDestroyed) {
                        plateSolveStatus?.text =
                            if (error is InterruptedException) "Résolution en ligne interrompue."
                            else "Échec du plate solving en ligne : ${error.message ?: "erreur réseau"}."
                        updatePlateSolverControls()
                    }
                }
            }
        }, "astrometry-online-solve")
        onlinePlateSolveThread = worker
        worker.start()
    }

    private fun saveAstrometryApiKey() {
        val apiKey = plateApiKeyField?.text?.toString()?.trim().orEmpty()
        if (apiKey.isBlank()) {
            plateApiKeyStatus?.text = "Saisis une clé API avant de l'enregistrer."
            return
        }
        try {
            AstrometryApiKeyStore(this).save(apiKey)
            storedAstrometryApiKey = apiKey
            plateApiKeyStatus?.text = "Clé enregistrée et chiffrée sur cet appareil."
            screenContainer.findViewWithTag<Button>(PLATE_API_KEY_CLEAR_BUTTON_TAG)?.isEnabled = true
            updatePlateSolverControls()
        } catch (error: Exception) {
            plateApiKeyStatus?.text = "Enregistrement de la clé impossible : ${error.message ?: "erreur Android Keystore"}."
        }
    }

    private fun clearAstrometryApiKey() {
        try {
            AstrometryApiKeyStore(this).clear()
            storedAstrometryApiKey = null
            plateApiKeyField?.text?.clear()
            plateApiKeyStatus?.text = "Clé enregistrée supprimée de cet appareil."
            screenContainer.findViewWithTag<Button>(PLATE_API_KEY_CLEAR_BUTTON_TAG)?.isEnabled = false
            updatePlateSolverControls()
        } catch (error: Exception) {
            plateApiKeyStatus?.text = "Suppression de la clé impossible : ${error.message ?: "erreur Android"}."
        }
    }

    private fun displayPlateImage(file: File) {
        val bounds = BitmapFactory.Options().apply { inJustDecodeBounds = true }
        BitmapFactory.decodeFile(file.absolutePath, bounds)
        if (bounds.outWidth <= 0 || bounds.outHeight <= 0) {
            plateImageLabel?.text = "Image illisible ou format non pris en charge."
            plateImage = null
            updatePlateSolverControls()
            return
        }
        val sample = (maxOf(bounds.outWidth / 1200, bounds.outHeight / 900, 1)).let {
            Integer.highestOneBit(it)
        }
        val bitmap = BitmapFactory.decodeFile(
            file.absolutePath,
            BitmapFactory.Options().apply { inSampleSize = sample }
        )
        if (bitmap == null) {
            plateImageLabel?.text = "Impossible de créer l'aperçu de cette image."
            plateImage = null
            updatePlateSolverControls()
            return
        }
        plateImage = file
        plateImagePreview?.setImageBitmap(bitmap)
        plateImageLabel?.text = "${file.name} · ${bounds.outWidth} × ${bounds.outHeight} · ${formatBytes(file.length())}"
        plateSolveStatus?.text = "Image prête à être résolue."
        updatePlateSolverControls()
    }

    private fun showConfiguration() {
        section("Encodeurs")
        val ticksAz = input("Pas par tour AZ", model.ticksPerRevAz.toString())
        val ticksAlt = input("Pas par tour ALT", model.ticksPerRevAlt.toString())
        val reverseAz = CheckBox(this).apply {
            text = "Inverser le sens AZ"
            isChecked = model.reverseAz
            setTextColor(primaryText())
        }
        val reverseAlt = CheckBox(this).apply {
            text = "Inverser le sens ALT"
            isChecked = model.reverseAlt
            setTextColor(primaryText())
        }
        screenContainer.addView(ticksAz)
        screenContainer.addView(ticksAlt)
        screenContainer.addView(reverseAz)
        screenContainer.addView(reverseAlt)

        section("Lieu d'observation")
        val sites = try {
            model.allSites()
        } catch (error: Exception) {
            toast(error.message ?: "Impossible de lire les lieux")
            emptyList()
        }
        val siteNames = sites.map { it.name }
        val siteSpinner = spinner(siteNames.ifEmpty { listOf(model.siteName) }, model.siteName)
        selectedSiteSpinner = siteSpinner
        screenContainer.addView(siteSpinner)
        siteSpinner.onItemSelectedListener = simpleSelection { name ->
            sites.firstOrNull { it.name == name }?.let {
                siteLatitudeField?.setText(it.latitude.toString())
                siteLongitudeField?.setText(it.longitude.toString())
            }
        }
        val latitude = input("Latitude (degrés Nord)", model.latitude.toString())
        val longitude = input("Longitude (degrés Est)", model.longitude.toString())
        siteLatitudeField = latitude
        siteLongitudeField = longitude
        screenContainer.addView(latitude)
        screenContainer.addView(longitude)
        screenContainer.addView(button("Utiliser la position GPS du téléphone").apply { setOnClickListener { requestPhoneLocation() } })
        val newName = input("Nom du nouveau lieu", "")
        val newLatitude = input("Latitude du nouveau lieu", "")
        val newLongitude = input("Longitude du nouveau lieu", "")
        screenContainer.addView(newName)
        screenContainer.addView(newLatitude)
        screenContainer.addView(newLongitude)
        screenContainer.addView(button("Ajouter le nouveau lieu").apply {
            setOnClickListener {
                val site = ObservationSite(newName.text.toString().trim(), newLatitude.number() ?: Double.NaN, newLongitude.number() ?: Double.NaN)
                try {
                    model.addSite(site)
                    renderScreen(SCREEN_CONFIG)
                    toast("Lieu ajouté et sélectionné.")
                } catch (error: IllegalArgumentException) {
                    toast(error.message ?: "Coordonnées de lieu invalides.")
                }
            }
        })
        screenContainer.addView(button("Supprimer le lieu sélectionné").apply {
            setOnClickListener {
                val selectedName = siteSpinner.selectedItem?.toString() ?: return@setOnClickListener
                try {
                    model.deleteSite(selectedName)
                    renderScreen(SCREEN_CONFIG)
                    toast("Lieu supprimé.")
                } catch (error: IllegalArgumentException) {
                    toast(error.message ?: "Impossible de supprimer ce lieu.")
                }
            }
        })
        screenContainer.addView(button("Enregistrer les réglages").apply {
            setOnClickListener {
                val az = ticksAz.number()
                val alt = ticksAlt.number()
                val lat = latitude.number()
                val lon = longitude.number()
                if (az == null || alt == null || lat == null || lon == null) {
                    toast("Renseigne des nombres valides pour la résolution et les coordonnées.")
                    return@setOnClickListener
                }
                val name = siteSpinner.selectedItem?.toString()?.takeIf { it.isNotBlank() } ?: "Lieu personnalisé"
                try {
                    model.setConfiguration(az, alt, reverseAz.isChecked, reverseAlt.isChecked, ObservationSite(name, lat, lon))
                    renderScreen(SCREEN_CONFIG)
                    toast("Configuration enregistrée sur le téléphone.")
                } catch (error: IllegalArgumentException) {
                    toast(error.message ?: "Configuration invalide.")
                }
            }
        })
    }

    private fun showCalibration() {
        section("Étalonnage des encodeurs")
        screenContainer.addView(text(
            "Place le tube à l'horizontale sur un repère stable. Démarre la mesure, réalise un tour AZ complet, puis une rotation ALT de 90°.",
            14f,
            secondaryText(),
            false
        ))
        val azRaw = value("AZ mesuré : 0 ticks")
        val azAngle = value("Angle AZ : 0.0°")
        val altRaw = value("ALT mesuré : 0 ticks")
        val altAngle = value("Angle ALT : 0.0°")
        calibrationLabels = listOf(azRaw, azAngle, altRaw, altAngle)
        calibrationBaseline()
        screenContainer.addView(button("Démarrer l'étalonnage · remet le pointage à l'horizontale").apply {
            setOnClickListener { model.calibrationStart(); refreshLiveData(); toast("Étalonnage démarré.") }
        })
        section("Azimut · rotation complète")
        screenContainer.addView(azRaw)
        screenContainer.addView(azAngle)
        screenContainer.addView(button("Valider le tour complet AZ").apply {
            setOnClickListener {
                val ticks = model.finishAzimuthCalibration()
                if (ticks == null) toast("Aucun déplacement AZ mesurable depuis le début de l'étalonnage.")
                else {
                    toast("Résolution AZ enregistrée : %.0f ticks/tour".format(ticks))
                    refreshLiveData()
                }
            }
        })
        section("Altitude · basculement de 90°")
        screenContainer.addView(altRaw)
        screenContainer.addView(altAngle)
        screenContainer.addView(button("Valider le basculement ALT de 90°").apply {
            setOnClickListener {
                val ticks = model.finishAltitudeCalibration()
                if (ticks == null) toast("Aucun déplacement ALT mesurable depuis le début de l'étalonnage.")
                else {
                    toast("Résolution ALT enregistrée : %.0f ticks/tour".format(ticks))
                    refreshLiveData()
                }
            }
        })
        screenContainer.addView(button("Terminer l'étalonnage").apply {
            setOnClickListener { model.endCalibration(); toast("Étalonnage terminé.") }
        })
    }

    private fun calibrationBaseline() {
        val labels = calibrationLabels
        if (labels.size == 4) {
            val azDelta = model.calibrationDeltaAz()
            val altDelta = model.calibrationDeltaAlt()
            labels[0].text = "AZ mesuré : $azDelta ticks"
            labels[1].text = "Angle AZ : %.1f°".format(azDelta / model.ticksPerRevAz * 360)
            labels[2].text = "ALT mesuré : $altDelta ticks"
            labels[3].text = "Angle ALT : %.1f°".format(altDelta / model.ticksPerRevAlt * 360)
        }
    }

    private fun showSimulation() {
        section("Simulation locale des encodeurs")
        screenContainer.addView(text(
            "Ces déplacements sont simulés dans l'application uniquement ; ils ne modifient pas les capteurs ni l'ESP32.",
            14f,
            secondaryText(),
            false
        ))
        val steps = listOf(-90.0, -45.0, -10.0, -1.0, -0.1, 0.1, 1.0, 10.0, 45.0, 90.0)
        listOf("AZ", "ALT").forEach { axis ->
            section("Axe $axis")
            steps.chunked(2).forEach { pair ->
                val row = LinearLayout(this).apply { orientation = LinearLayout.HORIZONTAL }
                pair.forEach { amount ->
                    row.addView(button("%+.1f°".format(amount)).apply {
                        setOnClickListener {
                            model.stepSimulation(axis, amount)
                            refreshLiveData()
                        }
                    }, LinearLayout.LayoutParams(0, dp(46), 1f).apply { marginEnd = dp(5) })
                }
                screenContainer.addView(row)
            }
        }
        screenContainer.addView(button("Remettre les deux axes simulés à zéro").apply {
            setOnClickListener { model.resetSimulation(); refreshLiveData() }
        })
    }

    private fun showSkySafari() {
        section("SkySafari · serveur TCP")
        skySafariStatusLabel = value(loadSkySafariStatus())
        screenContainer.addView(skySafariStatusLabel)
        screenContainer.addView(button(if (isSkySafariServiceRunning()) "Arrêter le serveur SkySafari" else "Démarrer le serveur SkySafari").apply {
            setOnClickListener {
                if (isSkySafariServiceRunning()) {
                    stopService(Intent(this@MobileMainActivity, SkySafariService::class.java))
                    refreshLiveData()
                } else {
                    requestPermissionsThenStartSkySafari()
                }
            }
        })
        screenContainer.addView(text(
            "Le téléphone et SkySafari doivent être connectés au même réseau Wi-Fi. Le Bluetooth reste connecté à l'ESP32. Si SkySafari tourne sur ce téléphone, utilise 127.0.0.1 ; depuis un autre appareil, utilise l'adresse Wi-Fi affichée ci-dessus.",
            14f,
            secondaryText(),
            false
        ))
        screenContainer.addView(text("Dans SkySafari : Basic Encoder System · Alt-Az Push-To · Wi-Fi/Ethernet · port 4030.", 14f, primaryText(), false))
    }

    private fun showHelp() {
        section("Connexion à l'ESP32")
        screenContainer.addView(text(
            "1. Alimente l'ESP32 combiné Web/BLE. 2. Active le Bluetooth du téléphone. 3. Dans l'application, choisis « Rechercher l'ESP32 » et autorise les demandes Android. 4. Sélectionne « Dobson Push-To ». Le site Web Wi-Fi reste disponible en parallèle.",
            15f,
            primaryText(),
            false
        ))
        section("Configuration SkySafari")
        screenContainer.addView(text(
            "Le même ESP32 fournit le site Web sur son point d'accès Wi-Fi et envoie les compteurs au téléphone par BLE. Les deux modes restent actifs en parallèle. Pour SkySafari, l'application Android ouvre un serveur TCP sur le port 4030 ; le téléphone et SkySafari doivent pouvoir se joindre sur le même réseau Wi-Fi.",
            14f,
            secondaryText(),
            false
        ))
        screenContainer.addView(button("Ouvrir les réglages SkySafari").apply { setOnClickListener { renderScreen(SCREEN_SKYSAFARI) } })
        section("Précision du pointage")
        screenContainer.addView(text(
            "Règle d'abord le lieu et la résolution des encodeurs, puis calibre chaque axe. Pour la mise en station, nivelle le tube et recale AZ/ALT, ou effectue l'alignement sur une ou deux étoiles. Les coordonnées du catalogue sont précessées et corrigées de la réfraction atmosphérique.",
            14f,
            secondaryText(),
            false
        ))
        screenContainer.addView(button("Configuration").apply { setOnClickListener { renderScreen(SCREEN_CONFIG) } })
        screenContainer.addView(button("Calibration").apply { setOnClickListener { renderScreen(SCREEN_CALIBRATION) } })
        screenContainer.addView(button("Mise en station").apply { setOnClickListener { renderScreen(SCREEN_STATION) } })
    }

    private fun showTests() {
        section("Tests locaux")
        val results = listOf(
            "Catalogue astronomique" to runCatching { objects.size >= 125 },
            "Résolutions des encodeurs" to runCatching { model.ticksPerRevAz > 0 && model.ticksPerRevAlt > 0 },
            "Coordonnées du lieu" to runCatching { model.latitude in -90.0..90.0 && model.longitude in -180.0..180.0 },
            "Calcul astronomique" to runCatching {
                val horizontal = Astronomy.toHorizontal(5.0, 20.0, model.latitude, model.longitude)
                horizontal.azimuth.isFinite() && horizontal.altitude.isFinite()
            },
            "Lien BLE ESP32" to runCatching { model.connected }
        )
        results.forEach { (name, result) ->
            screenContainer.addView(value("$name : ${if (result.getOrDefault(false)) "OK" else "À vérifier"}"))
            result.exceptionOrNull()?.let { screenContainer.addView(text(it.message ?: "Échec du test", 13f, errorColor(), false)) }
        }
        screenContainer.addView(button("Relancer les tests").apply { setOnClickListener { renderScreen(SCREEN_TESTS) } })
        screenContainer.addView(button("Rechercher l'ESP32").apply { setOnClickListener { ensurePermissionsThenScan() {} } })
    }

    private fun showReleaseNotes() {
        section("Version mobile 0.7.0")
        listOf(
            "Connexion Bluetooth LE aux encodeurs ESP32.",
            "Pointage AZ/ALT, affichage des compteurs et recalages mémorisés sur le téléphone.",
            "Catalogue Messier et NGC avec planètes, filtres par type et magnitude, calcul de visibilité et guidage.",
            "Mise en station rapide et alignement guidé sur une ou deux étoiles.",
            "Configuration des encodeurs, lieux d'observation et GPS du téléphone.",
            "Calibration des deux axes, simulation locale et visée céleste 3D par capteurs.",
            "Serveur TCP SkySafari intégré à l'application Android sur le port 4030.",
            "Mode nuit rouge mémorisé et aide intégrée.",
            "Viseur céleste orienté par les capteurs du téléphone, avec un mode réalité augmentée par caméra.",
            "Navigation inférieure simplifiée et interface revue avec cartes arrondies et contraste renforcé.",
            "Plate solving en ligne via Astrometry.net, avec confirmation avant l'envoi privé de l'image.",
            "Clé API Astrometry.net enregistrable localement avec chiffrement Android Keystore.",
            "Suppression du solveur local et de ses index.",
            "Raccourci de mode nuit en bas, avec contraste inversé.",
            "Vue céleste orientée par la boussole et réalité augmentée avec caméra."
        ).forEach { screenContainer.addView(value("• $it")) }
        screenContainer.addView(text(
            "Le firmware ESP32 combiné conserve l'interface Web et le service BLE simultanément ; les deux partagent les encodeurs et les réglages matériels.",
            14f,
            secondaryText(),
            false
        ))
    }

    private fun refreshLiveData() {
        if (!::statusLabel.isInitialized) return
        val snapshot = model.snapshot()
        if (!model.connected) {
            statusLabel.text = "Bluetooth déconnecté · position figée au dernier échantillon"
        } else {
            statusLabel.text = "Bluetooth connecté · télémétrie reçue"
        }
        liveAzLabel?.text = "Azimut : %.1f°".format(snapshot.azDegrees)
        liveAltLabel?.text = "Altitude : %.1f°".format(snapshot.altDegrees)
        liveTicksLabel?.text = "Ticks AZ : ${snapshot.azTicks}   ·   Ticks ALT : ${snapshot.altTicks}"
        val target = model.activeTarget
        if (target != null) {
            val now = System.currentTimeMillis()
            val (ra, dec) = Astronomy.precess(target.ra, target.dec, now)
            val horizontal = Astronomy.toHorizontal(ra, dec, model.latitude, model.longitude, now)
            val deltaAz = Astronomy.angularDifference(horizontal.azimuth, snapshot.azDegrees)
            val deltaAlt = horizontal.altitude - snapshot.altDegrees
            liveTargetLabel?.text = "${target.commonName.ifBlank { target.name }} · AZ %+.1f° · ALT %+.1f° · écart %+.1f° / %+.1f°".format(
                horizontal.azimuth,
                horizontal.altitude,
                deltaAz,
                deltaAlt
            )
            guidanceView?.apply {
                azimuthError = deltaAz
                altitudeError = deltaAlt
                nightMode = model.nightMode
            }
        } else {
            liveTargetLabel?.text = "Aucune cible sélectionnée"
            guidanceView?.apply {
                azimuthError = 0.0
                altitudeError = 0.0
                nightMode = model.nightMode
            }
        }
        skyDome?.apply {
            azimuth = snapshot.azDegrees
            altitude = snapshot.altDegrees
            latitude = model.latitude
            longitude = model.longitude
            nightMode = model.nightMode
            sensorConnected = model.connected
            lastTelemetryAt = model.lastTelemetryAt
            selectedObject = target
            phoneAzimuth = this@MobileMainActivity.phoneAzimuth
            phoneAltitude = this@MobileMainActivity.phoneAltitude
            invalidate()
        }
        if (!gpsTrackingEnabled) {
            gpsStatusLabel?.text =
                "${model.siteName} · %.4f°, %.4f°".format(model.latitude, model.longitude)
        }
        skySafariStatusLabel?.text = loadSkySafariStatus()
        if (activeScreen == SCREEN_CALIBRATION) {
            calibrationLabels.getOrNull(0)?.text = "AZ mesuré : ${model.calibrationDeltaAz()} ticks"
            calibrationLabels.getOrNull(1)?.text = "Angle AZ estimé : %.1f°".format(model.calibrationDeltaAz() / model.ticksPerRevAz * 360)
            calibrationLabels.getOrNull(2)?.text = "ALT mesuré : ${model.calibrationDeltaAlt()} ticks"
            calibrationLabels.getOrNull(3)?.text = "Angle ALT estimé : %.1f°".format(model.calibrationDeltaAlt() / model.ticksPerRevAlt * 360)
        }
    }

    private fun alignToSelectedTarget() {
        val target = model.activeTarget
        if (target == null) {
            toast("Choisis une cible dans le catalogue avant de l'aligner.")
            return
        }
        val (ra, dec) = Astronomy.precess(target.ra, target.dec)
        val horizontal = Astronomy.toHorizontal(ra, dec, model.latitude, model.longitude)
        model.alignTo(horizontal.azimuth, horizontal.altitude)
        refreshLiveData()
        toast("Pointage aligné sur ${target.commonName.ifBlank { target.name }}.")
    }

    private fun ensurePermissionsThenScan(onStarted: () -> Unit) {
        val permissions = if (Build.VERSION.SDK_INT >= 31) {
            arrayOf(Manifest.permission.BLUETOOTH_SCAN, Manifest.permission.BLUETOOTH_CONNECT)
        } else {
            arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
        }
        requestMissing(permissions, REQUEST_BLE_PERMISSIONS) { onStarted(); beginScan() }
    }

    private fun beginScan() {
        if (scanning) {
            toast("La recherche BLE est déjà en cours.")
            return
        }
        if (!bluetoothAdapter.isEnabled) {
            toast("Active le Bluetooth du téléphone puis réessaie.")
            return
        }
        val bleScanner = bluetoothAdapter.bluetoothLeScanner
        if (bleScanner == null) {
            toast("Le scanner Bluetooth LE est indisponible.")
            return
        }
        scanner = bleScanner
        discoveredDevices.clear()
        screenContainer.findViewWithTag<LinearLayout>(DEVICE_LIST_TAG)?.removeAllViews()
        scanning = true
        try {
            val filter = ScanFilter.Builder().setServiceUuid(ParcelUuid(serviceUuid)).build()
            val settings = ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build()
            bleScanner.startScan(listOf(filter), settings, scanCallback)
            handler.postDelayed(stopScanTask, SCAN_DURATION_MS)
            toast("Recherche de Dobson Push-To…")
        } catch (error: SecurityException) {
            scanning = false
            toast("Permission Bluetooth manquante : ${error.message ?: "accès refusé"}.")
        } catch (error: RuntimeException) {
            scanning = false
            toast("La recherche BLE a échoué : ${error.message ?: "erreur Android"}.")
        }
    }

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            runOnUiThread { addDevice(result.device) }
        }

        override fun onScanFailed(errorCode: Int) {
            runOnUiThread {
                scanning = false
                toast("La recherche BLE a échoué (code $errorCode).")
            }
        }
    }

    private fun addDevice(device: BluetoothDevice) {
        val address = try {
            device.address
        } catch (error: SecurityException) {
            toast("Permission Bluetooth manquante pour lire les appareils.")
            return
        } ?: return
        if (discoveredDevices.containsKey(address)) return
        discoveredDevices[address] = device
        val name = try {
            device.name ?: "Appareil BLE"
        } catch (_: SecurityException) {
            "Appareil BLE"
        }
        val list = screenContainer.findViewWithTag<LinearLayout>(DEVICE_LIST_TAG)
        if (list != null) {
            list.addView(button("$name · $address").apply {
                setOnClickListener { stopScan(); connect(device) }
            })
        } else {
            stopScan()
            AlertDialog.Builder(this)
                .setTitle("ESP32 détecté")
                .setMessage("$name\n$address")
                .setNegativeButton("Ignorer", null)
                .setPositiveButton("Connecter") { _, _ -> connect(device) }
                .show()
        }
    }

    private val stopScanTask = Runnable { stopScan() }

    private fun stopScan() {
        if (!scanning) return
        try {
            scanner?.stopScan(scanCallback)
        } catch (error: SecurityException) {
            toast("Permission Bluetooth manquante pour arrêter la recherche.")
        }
        scanning = false
        handler.removeCallbacks(stopScanTask)
        if (discoveredDevices.isEmpty()) toast("Aucun appareil détecté. Vérifie que l'ESP32 est alimenté.")
    }

    private fun connect(device: BluetoothDevice) {
        disconnect()
        try {
            gatt = device.connectGatt(this, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
            statusLabel.text = "Connexion Bluetooth à l'ESP32…"
        } catch (error: SecurityException) {
            toast("Permission Bluetooth manquante : ${error.message ?: "accès refusé"}.")
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                runOnUiThread {
                    model.connected = false
                    statusLabel.text = "Connexion BLE interrompue (code $status)."
                }
                gatt.close()
                return
            }
            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    model.connected = true
                    runOnUiThread { statusLabel.text = "Connecté · découverte du service de télémétrie…" }
                    try {
                        gatt.discoverServices()
                    } catch (error: SecurityException) {
                        runOnUiThread { toast("Permission Bluetooth manquante : ${error.message ?: "accès refusé"}.") }
                    }
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    model.connected = false
                    runOnUiThread { refreshLiveData() }
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                runOnUiThread { toast("Impossible de découvrir les services BLE.") }
                return
            }
            val characteristic = gatt.getService(serviceUuid)?.getCharacteristic(telemetryUuid)
            if (characteristic == null) {
                runOnUiThread { toast("Le service Dobson Push-To attendu n'est pas présent.") }
                return
            }
            try {
                if (!gatt.setCharacteristicNotification(characteristic, true)) {
                    runOnUiThread { toast("Impossible d'activer les notifications BLE.") }
                    return
                }
                val descriptor = characteristic.getDescriptor(cccdUuid)
                if (descriptor == null) {
                    runOnUiThread { toast("Descripteur de notifications BLE absent.") }
                    return
                }
                if (Build.VERSION.SDK_INT >= 33) {
                    val result = gatt.writeDescriptor(descriptor, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)
                    if (result != BluetoothStatusCodes.SUCCESS) {
                        runOnUiThread { toast("Activation des notifications BLE échouée.") }
                    }
                } else {
                    @Suppress("DEPRECATION")
                    descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                    @Suppress("DEPRECATION")
                    if (!gatt.writeDescriptor(descriptor)) {
                        runOnUiThread { toast("Activation des notifications BLE échouée.") }
                    }
                }
            } catch (error: SecurityException) {
                runOnUiThread { toast("Permission Bluetooth manquante : ${error.message ?: "accès refusé"}.") }
            }
        }

        @Deprecated("Android 12 and earlier")
        override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
            if (characteristic.uuid == telemetryUuid) decodeTelemetry(characteristic.value)
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray
        ) {
            if (characteristic.uuid == telemetryUuid) decodeTelemetry(value)
        }
    }

    private fun decodeTelemetry(bytes: ByteArray?) {
        if (bytes == null || (bytes.size != 8 && bytes.size != 16)) {
            runOnUiThread { toast("Trame BLE invalide : 8 octets de compteurs attendus.") }
            return
        }
        val data = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN)
        val azTicks = data.int
        val altTicks = data.int
        model.acceptTelemetry(azTicks, altTicks)
        runOnUiThread { refreshLiveData() }
    }

    private fun disconnect() {
        val current = gatt ?: return
        try {
            current.disconnect()
            current.close()
        } catch (error: SecurityException) {
            toast("Permission Bluetooth manquante : ${error.message ?: "accès refusé"}.")
        }
        gatt = null
        model.connected = false
        refreshLiveData()
    }

    private fun requestPermissionsThenStartSkySafari() {
        val needed = mutableListOf<String>()
        if (Build.VERSION.SDK_INT >= 31 && checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            needed.add(Manifest.permission.BLUETOOTH_CONNECT)
        }
        if (Build.VERSION.SDK_INT >= 33 && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED) {
            needed.add(Manifest.permission.POST_NOTIFICATIONS)
        }
        requestMissing(needed.toTypedArray(), REQUEST_SKYSAFARI_PERMISSIONS) {
            startSkySafariService()
        }
    }

    private fun startSkySafariService() {
        val intent = Intent(this, SkySafariService::class.java).setAction(SkySafariService.ACTION_START)
        try {
            if (Build.VERSION.SDK_INT >= 26) startForegroundService(intent) else startService(intent)
            toast("Serveur SkySafari démarré.")
        } catch (error: Exception) {
            toast("Impossible de démarrer SkySafari : ${error.message ?: "erreur Android"}.")
        }
    }

    private fun isSkySafariServiceRunning(): Boolean =
        getSharedPreferences("dobson-mobile", MODE_PRIVATE).getBoolean("skySafariRunning", false)

    private fun loadSkySafariStatus(): String =
        getSharedPreferences("dobson-mobile", MODE_PRIVATE)
            .getString("skySafariStatus", "Serveur SkySafari arrêté.")
            ?: "Serveur SkySafari arrêté."

    private fun requestPhoneLocation() {
        val needed = arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
        requestMissing(needed, REQUEST_LOCATION_PERMISSION) {
            val locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager
            val providers = listOf(LocationManager.GPS_PROVIDER, LocationManager.NETWORK_PROVIDER)
            val location = providers.firstNotNullOfOrNull { provider ->
                try {
                    locationManager.getLastKnownLocation(provider)
                } catch (_: SecurityException) {
                    null
                } catch (_: IllegalArgumentException) {
                    null
                }
            }
            if (location != null) {
                applyLocation(location)
                return@requestMissing
            }
            val provider = providers.firstOrNull {
                try {
                    locationManager.isProviderEnabled(it)
                } catch (_: Exception) {
                    false
                }
            }
            if (provider == null) {
                toast("Active la localisation du téléphone et réessaie.")
                return@requestMissing
            }
            pendingGpsResult = { applyLocation(it) }
            val listener = object : LocationListener {
                override fun onLocationChanged(location: Location) {
                    locationListener = null
                    pendingGpsResult?.invoke(location)
                    pendingGpsResult = null
                    try {
                        locationManager.removeUpdates(this)
                    } catch (_: Exception) {
                    }
                }
            }
            locationListener = listener
            try {
                locationManager.requestSingleUpdate(provider, listener, Looper.getMainLooper())
                toast("Recherche de la position GPS…")
                handler.postDelayed({
                    if (locationListener === listener) {
                        pendingGpsResult = null
                        locationListener = null
                        try {
                            locationManager.removeUpdates(listener)
                        } catch (_: Exception) {
                        }
                        toast("Position GPS indisponible ; vérifie les permissions et réessaie.")
                    }
                }, 12_000L)
            } catch (error: Exception) {
                pendingGpsResult = null
                locationListener = null
                toast("Lecture GPS impossible : ${error.message ?: "erreur Android"}.")
            }
        }
    }

    private fun applyLocation(location: Location) {
        val site = ObservationSite("GPS Mobile", location.latitude, location.longitude)
        model.setSite(site)
        siteLatitudeField?.setText(location.latitude.toString())
        siteLongitudeField?.setText(location.longitude.toString())
        val accuracy = if (location.hasAccuracy()) " · précision ±%.0f m".format(location.accuracy) else ""
        gpsStatusLabel?.text =
            "GPS · %.5f°, %.5f°$accuracy".format(location.latitude, location.longitude)
        refreshLiveData()
        toast("Position GPS récupérée et appliquée au lieu d'observation.")
    }

    private fun requestMissing(permissions: Array<String>, requestCode: Int, granted: () -> Unit) {
        val missing = permissions.filter { checkSelfPermission(it) != PackageManager.PERMISSION_GRANTED }
        if (missing.isEmpty()) {
            granted()
        } else {
            pendingPermissionAction = granted
            pendingPermissionCode = requestCode
            requestPermissions(missing.toTypedArray(), requestCode)
        }
    }

    private var pendingPermissionAction: (() -> Unit)? = null
    private var pendingPermissionCode = 0

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode != pendingPermissionCode) return
        val action = pendingPermissionAction
        pendingPermissionAction = null
        if (grantResults.isNotEmpty() && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
            action?.invoke()
        } else {
            toast("Permission refusée. Autorise-la dans les paramètres Android pour utiliser cette fonction.")
        }
    }

    private fun validateHorizontal(azimuth: Double?, altitude: Double?): Boolean {
        if (azimuth == null || altitude == null || !azimuth.isFinite() || !altitude.isFinite() ||
            azimuth !in 0.0..360.0 || altitude !in -90.0..90.0
        ) {
            toast("Saisis un azimut entre 0 et 360° et une altitude entre -90 et 90°.")
            return false
        }
        return true
    }

    private fun spinner(options: List<String>, selected: String): Spinner {
        val control = Spinner(this)
        control.adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, options)
        val index = options.indexOf(selected)
        if (index >= 0) control.setSelection(index)
        return control
    }

    private fun magnitudeLabel(limit: Double): String = when (limit) {
        6.0 -> "≤ 6 · œil nu"
        8.0 -> "≤ 8 · jumelles"
        10.0 -> "≤ 10 · petit télescope"
        12.0 -> "≤ 12 · grand télescope"
        else -> "Toutes magnitudes"
    }

    private fun section(label: String) {
        val heading = text(label, 18f, accentColor(), true)
        heading.setPadding(0, dp(15), 0, dp(6))
        screenContainer.addView(heading)
    }

    private fun value(label: String): TextView {
        val view = text(label, 16f, primaryText(), false)
        view.setPadding(dp(14), dp(12), dp(14), dp(12))
        view.background = roundedBackground(cardColor(), dp(12), borderColor())
        view.layoutParams = LinearLayout.LayoutParams(-1, -2).apply { bottomMargin = dp(5) }
        return view
    }

    private fun text(label: String, size: Float, color: Int, bold: Boolean): TextView =
        TextView(this).apply {
            text = label
            textSize = size
            setTextColor(color)
            if (bold) setTypeface(typeface, android.graphics.Typeface.BOLD)
            setPadding(dp(2), dp(4), dp(2), dp(4))
        }

    private fun button(label: String): Button =
        Button(this).apply {
            text = label
            isAllCaps = false
            setTextColor(primaryText())
            textSize = 14f
            background = roundedBackground(
                if (model.nightMode) Color.rgb(80, 14, 14) else Color.rgb(28, 45, 65),
                dp(12),
                borderColor()
            )
            minHeight = dp(48)
            stateListAnimator = null
            layoutParams = LinearLayout.LayoutParams(-1, -2).apply { bottomMargin = dp(6) }
        }

    private fun card(): LinearLayout =
        LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(dp(14), dp(10), dp(14), dp(10))
            background = roundedBackground(cardColor(), dp(14), borderColor())
            layoutParams = LinearLayout.LayoutParams(-1, -2).apply { bottomMargin = dp(8) }
        }

    private fun input(hintText: String, initial: String): EditText =
        EditText(this).apply {
            hint = hintText
            if (initial.isNotBlank()) setText(initial)
            setTextColor(primaryText())
            setHintTextColor(secondaryText())
            setSingleLine(true)
            layoutParams = LinearLayout.LayoutParams(-1, -2).apply { bottomMargin = dp(4) }
        }

    private fun EditText.number(): Double? =
        text.toString().trim().replace(',', '.').toDoubleOrNull()

    private fun simpleSelection(action: (String) -> Unit) =
        object : android.widget.AdapterView.OnItemSelectedListener {
            override fun onNothingSelected(parent: android.widget.AdapterView<*>?) = Unit
            override fun onItemSelected(
                parent: android.widget.AdapterView<*>?,
                view: View?,
                position: Int,
                id: Long
            ) {
                parent?.getItemAtPosition(position)?.toString()?.let(action)
            }
        }

    private fun simpleTextChange(action: (String) -> Unit) =
        object : android.text.TextWatcher {
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) = Unit
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                action(s?.toString().orEmpty())
            }
            override fun afterTextChanged(s: android.text.Editable?) = Unit
        }

    private fun showError(message: String) {
        setContentView(LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            gravity = Gravity.CENTER
            setPadding(dp(24), dp(24), dp(24), dp(24))
            setBackgroundColor(Color.rgb(18, 18, 18))
            addView(text(message, 18f, Color.WHITE, false))
        })
    }

    private fun toast(message: String) = Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
    private fun dp(value: Int): Int = (value * resources.displayMetrics.density).toInt()
    private fun matchWidth() = LinearLayout.LayoutParams(-1, -2)
    private fun backgroundColor() = if (model.nightMode) Color.rgb(22, 0, 0) else Color.rgb(10, 17, 28)
    private fun cardColor() = if (model.nightMode) Color.rgb(45, 5, 5) else Color.rgb(21, 32, 48)
    private fun primaryText() = if (model.nightMode) Color.rgb(255, 190, 190) else Color.WHITE
    private fun secondaryText() = if (model.nightMode) Color.rgb(215, 125, 125) else Color.rgb(165, 183, 207)
    private fun accentColor() = if (model.nightMode) Color.rgb(255, 92, 92) else Color.rgb(101, 213, 255)
    private fun errorColor() = Color.rgb(255, 82, 82)
    private fun borderColor() = if (model.nightMode) Color.rgb(112, 28, 39) else Color.rgb(45, 67, 91)

    private fun roundedBackground(color: Int, radius: Int, stroke: Int) =
        GradientDrawable().apply {
            setColor(color)
            cornerRadius = radius.toFloat()
            setStroke(dp(1), stroke)
        }

    private fun formatBytes(bytes: Long): String =
        if (bytes >= 1024L * 1024L) "%.1f Mo".format(bytes / (1024.0 * 1024.0))
        else "%.0f Ko".format(bytes / 1024.0)

    override fun onPause() {
        if (gpsTrackingEnabled) stopGpsTracking(clearRequested = false)
        stopSkySensors()
        skyCameraPreview?.stop()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        if (gpsTrackingEnabled && activeScreen == SCREEN_SKY_3D && gpsTrackingListener == null) {
            startGpsTracking()
        }
        if (activeScreen == SCREEN_SKY_3D) {
            startSkySensors()
            if (skyArMode && checkSelfPermission(Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
                skyCameraPreview?.start()
            }
        }
    }

    override fun onDestroy() {
        handler.removeCallbacks(telemetryRefresh)
        onlinePlateSolveThread?.interrupt()
        stopSkySensors()
        stopSkyCameraPreview()
        stopScan()
        disconnect()
        stopGpsTracking()
        try {
            locationListener?.let {
                (getSystemService(Context.LOCATION_SERVICE) as LocationManager).removeUpdates(it)
            }
        } catch (_: Exception) {
        }
        super.onDestroy()
    }

    companion object {
        private const val SCREEN_HOME = "home"
        private const val SCREEN_CATALOG = "catalog"
        private const val SCREEN_STATION = "station"
        private const val SCREEN_SKY_3D = "sky3d"
        private const val SCREEN_CONFIG = "config"
        private const val SCREEN_CALIBRATION = "calibration"
        private const val SCREEN_SIMULATION = "simulation"
        private const val SCREEN_SKYSAFARI = "skysafari"
        private const val SCREEN_PLATE_SOLVER = "plate-solver"
        private const val SCREEN_HELP = "help"
        private const val SCREEN_TESTS = "tests"
        private const val SCREEN_RELEASES = "releases"
        private const val SCREEN_MORE = "more"
        private const val SCREEN_NIGHT = "night-mode"
        private const val DEVICE_LIST_TAG = "ble-devices"
        private const val CATALOG_ROWS_TAG = "catalog-rows"
        private const val PLATE_ONLINE_BUTTON_TAG = "plate-online-button"
        private const val PLATE_API_KEY_CLEAR_BUTTON_TAG = "plate-api-key-clear-button"
        private const val PLATE_ALIGN_BUTTON_TAG = "plate-align-button"
        private const val REQUEST_BLE_PERMISSIONS = 70
        private const val REQUEST_SKYSAFARI_PERMISSIONS = 71
        private const val REQUEST_LOCATION_PERMISSION = 72
        private const val REQUEST_PICK_PLATE_IMAGE = 73
        private const val REQUEST_CAPTURE_PLATE_IMAGE = 74
        private const val REQUEST_CAMERA_PERMISSION = 75
        private const val SCAN_DURATION_MS = 12_000L
        private const val TELEMETRY_REFRESH_MS = 250L
        private val PRIMARY_NAVIGATION = listOf(
            SCREEN_HOME to "Accueil",
            SCREEN_CATALOG to "Objets",
            SCREEN_STATION to "Alignement",
            SCREEN_SKY_3D to "Vue 3D",
            SCREEN_MORE to "Plus",
            SCREEN_NIGHT to "Mode nuit"
        )
        private val SCREEN_LABELS = linkedMapOf(
            SCREEN_HOME to "Accueil",
            SCREEN_CATALOG to "Catalogue",
            SCREEN_STATION to "Alignement",
            SCREEN_SKY_3D to "Vue 3D",
            SCREEN_PLATE_SOLVER to "Plate solving",
            SCREEN_CONFIG to "Configuration",
            SCREEN_CALIBRATION to "Calibration",
            SCREEN_SIMULATION to "Simulation",
            SCREEN_SKYSAFARI to "SkySafari",
            SCREEN_HELP to "Aide",
            SCREEN_TESTS to "Tests",
            SCREEN_RELEASES to "Notes"
        )
    }
}
