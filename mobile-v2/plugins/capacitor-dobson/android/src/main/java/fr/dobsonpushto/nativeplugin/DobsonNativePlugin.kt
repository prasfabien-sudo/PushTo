package fr.dobsonpushto.nativeplugin

import android.content.Context
import android.Manifest
import android.content.ContentValues
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.net.wifi.WifiNetworkSpecifier
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.os.Environment
import android.provider.MediaStore
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Intent
import android.content.IntentFilter
import android.graphics.Bitmap
import android.graphics.Color
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbManager
import com.zwo.ASICameraProperty
import com.zwo.ASIConstants
import com.zwo.ASIImageBuffer
import com.zwo.ASIUSBManager
import com.zwo.ZwoCamera
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import com.getcapacitor.JSArray
import com.getcapacitor.JSObject
import com.getcapacitor.Plugin
import com.getcapacitor.PluginCall
import com.getcapacitor.PluginMethod
import com.getcapacitor.PermissionState
import com.getcapacitor.annotation.CapacitorPlugin
import com.getcapacitor.annotation.Permission
import com.getcapacitor.annotation.PermissionCallback
import org.json.JSONArray
import org.json.JSONObject
import java.security.KeyStore
import java.io.File
import java.io.ByteArrayOutputStream
import java.io.EOFException
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.net.InetSocketAddress
import java.net.Socket
import java.util.concurrent.Executors
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

@CapacitorPlugin(
    name = "DobsonNative",
    permissions = [Permission(alias = "photoStorage", strings = [Manifest.permission.WRITE_EXTERNAL_STORAGE])]
)
class DobsonNativePlugin : Plugin(), SensorEventListener {
    private var networkCallback: ConnectivityManager.NetworkCallback? = null
    private var pendingWifiCall: PluginCall? = null
    private var sensorManager: SensorManager? = null
    private var sensorThread: HandlerThread? = null
    private var sensorHandler: Handler? = null
    private var database: CatalogDatabase? = null
    private val encoderExecutor = Executors.newSingleThreadExecutor()
    private val zwoExecutor = Executors.newSingleThreadExecutor { runnable ->
        Thread(runnable, "zwo-camera").apply {
            setUncaughtExceptionHandler { _, error -> zwoLog("Erreur non capturée dans le thread caméra.", error) }
        }
    }
    private val zwoLogMaxBytes = 100_000L
    private val zwoVendorId = 0x03C3
    private val zwoSuccess = ASIConstants.ASI_ERROR_CODE.ASI_SUCCESS
    private val zwoPermissionAction = "fr.dobsonpushto.nativeplugin.ZWO_USB_PERMISSION"
    private var zwoCamera: ZwoCamera? = null
    private var zwoBuffer: ASIImageBuffer? = null
    private var zwoBufferSize = 0
    private var zwoCameraName = ""
    private var zwoWidth = 0
    private var zwoHeight = 0
    private var zwoIsColor = false
    private var zwoBayerPattern = 0
    private var zwoExposureMs = 100.0
    private var zwoGain = 100
    private var zwoPendingOpen: ZwoOpenRequest? = null
    private var zwoPermissionReceiver: BroadcastReceiver? = null

    private class ZwoOpenRequest(val call: PluginCall, val index: Int, val exposureMs: Double, val gain: Int)

    @PluginMethod
    fun saveAstroPhoto(call: PluginCall) {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.P &&
            getPermissionState("photoStorage") != PermissionState.GRANTED) {
            requestPermissionForAlias("photoStorage", call, "photoStoragePermissionResult")
            return
        }
        saveAstroPhotoWithPermission(call)
    }

    @PermissionCallback
    private fun photoStoragePermissionResult(call: PluginCall) {
        if (getPermissionState("photoStorage") != PermissionState.GRANTED) {
            call.reject("L'accès au stockage est nécessaire pour enregistrer la photo.")
            return
        }
        saveAstroPhotoWithPermission(call)
    }

    private fun saveAstroPhotoWithPermission(call: PluginCall) {
        val base64 = call.getString("base64")
        val filename = call.getString("filename")
        if (base64.isNullOrBlank() || filename.isNullOrBlank() ||
            !filename.matches(Regex("ciel-[0-9]+\\.jpg"))) {
            call.reject("Photo ou nom de fichier invalide.")
            return
        }
        val bytes = try {
            Base64.decode(base64, Base64.NO_WRAP)
        } catch (error: IllegalArgumentException) {
            call.reject("La photo est invalide.", error)
            return
        }
        if (bytes.isEmpty()) {
            call.reject("La photo est vide.")
            return
        }

        val resolver = context.contentResolver
        val values = ContentValues().apply {
            put(MediaStore.Images.Media.DISPLAY_NAME, filename)
            put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg")
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                put(MediaStore.Images.Media.RELATIVE_PATH, "${Environment.DIRECTORY_PICTURES}/AstroPhoto")
                put(MediaStore.Images.Media.IS_PENDING, 1)
            } else {
                @Suppress("DEPRECATION")
                val directory = File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "AstroPhoto")
                if (!directory.exists() && !directory.mkdirs()) {
                    call.reject("Impossible de créer le dossier AstroPhoto.")
                    return
                }
                @Suppress("DEPRECATION")
                put(MediaStore.Images.Media.DATA, File(directory, filename).absolutePath)
            }
        }
        var uri: android.net.Uri? = null
        try {
            uri = resolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values)
                ?: throw IOException("Impossible de créer la photo dans la galerie.")
            val output = resolver.openOutputStream(uri) ?: throw IOException("Impossible d'ouvrir la photo.")
            output.use { it.write(bytes) }
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                val published = ContentValues().apply { put(MediaStore.Images.Media.IS_PENDING, 0) }
                if (resolver.update(uri, published, null, null) != 1) {
                    throw IOException("Impossible de publier la photo dans la galerie.")
                }
            }
            call.resolve()
        } catch (error: Exception) {
            if (uri != null) {
                try {
                    resolver.delete(uri, null, null)
                } catch (cleanupError: Exception) {
                    android.util.Log.e("DobsonNative", "Nettoyage de la photo incomplète impossible.", cleanupError)
                }
            }
            call.reject("Impossible d'enregistrer la photo dans AstroPhoto.", error)
        }
    }

    @PluginMethod
    fun connect(call: PluginCall) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q) {
            call.reject("La connexion assistée au Wi-Fi nécessite Android 10 ou une version ultérieure.")
            return
        }

        val ssid = call.getString("ssid")?.trim().orEmpty()
        val password = call.getString("password").orEmpty()
        if (ssid.isBlank()) {
            call.reject("Le nom du réseau Wi-Fi est obligatoire.")
            return
        }
        if (password.isNotEmpty() && password.length < 8) {
            call.reject("Le mot de passe WPA2 doit contenir au moins 8 caractères.")
            return
        }

        val specifierBuilder = WifiNetworkSpecifier.Builder().setSsid(ssid)
        if (password.isNotEmpty()) specifierBuilder.setWpa2Passphrase(password)

        val request = NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
            .setNetworkSpecifier(specifierBuilder.build())
            .build()
        val connectivity = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

        releaseNetworkRequest(connectivity)
        pendingWifiCall = call
        val callback = object : ConnectivityManager.NetworkCallback() {
            override fun onAvailable(network: Network) {
                connectivity.bindProcessToNetwork(network)
                val result = JSObject()
                    .put("ssid", ssid)
                pendingWifiCall?.resolve(result)
                pendingWifiCall = null
                notifyListeners("wifiState", JSObject().put("connected", true).put("ssid", ssid))
            }

            override fun onLost(network: Network) {
                if (networkCallback !== this) return
                connectivity.bindProcessToNetwork(null)
                notifyListeners("wifiState", JSObject().put("connected", false).put("ssid", ssid))
            }

            override fun onUnavailable() {
                if (networkCallback !== this) return
                pendingWifiCall?.reject("Android n'a pas pu rejoindre le réseau Wi-Fi. Vérifiez le réseau et acceptez la demande système.")
                pendingWifiCall = null
                networkCallback = null
            }
        }

        try {
            networkCallback = callback
            connectivity.requestNetwork(request, callback, 30_000)
        } catch (error: SecurityException) {
            networkCallback = null
            pendingWifiCall = null
            call.reject("Android a refusé la demande d'accès au Wi-Fi.", error)
        } catch (error: RuntimeException) {
            networkCallback = null
            pendingWifiCall = null
            call.reject("La demande de connexion Wi-Fi a échoué.", error)
        }
    }

    @PluginMethod
    fun disconnect(call: PluginCall) {
        val connectivity = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        releaseNetworkRequest(connectivity)
        call.resolve()
    }

    @PluginMethod
    fun getEncoderStatus(call: PluginCall) {
        encoderExecutor.execute {
            try {
                Socket().use { socket ->
                    socket.connect(InetSocketAddress("192.168.4.1", 4030), 1500)
                    socket.soTimeout = 1500
                    val input = socket.getInputStream()
                    val output = socket.getOutputStream()
                    val position = readEncoderValues(input, output, 'Q')
                    val resolution = readEncoderValues(input, output, 'a')
                    if (resolution[0] <= 0 || resolution[1] <= 0) {
                        throw IOException("Résolution des encodeurs invalide.")
                    }
                    call.resolve(
                        JSObject()
                            .put("rawAZ", position[0])
                            .put("rawALT", position[1])
                            .put("ticksAZ", resolution[0])
                            .put("ticksALT", resolution[1])
                    )
                }
            } catch (error: Exception) {
                call.reject("Impossible de lire les encodeurs via TCP sur le port 4030.", error)
            }
        }
    }

    private fun readEncoderValues(input: InputStream, output: OutputStream, command: Char): List<Long> {
        output.write(command.code)
        output.flush()
        val response = ByteArrayOutputStream()
        while (response.size() < 64) {
            val value = input.read()
            if (value < 0) throw EOFException("Connexion encodeurs interrompue.")
            if (value == '\r'.code) break
            if (value != '\n'.code) response.write(value)
        }
        if (response.size() >= 64) throw IOException("Réponse encodeurs trop longue.")
        val values = response.toString(Charsets.UTF_8.name()).trim().split(Regex("\\s+"))
        if (values.size != 2) throw IOException("Réponse encodeurs invalide.")
        return values.map { it.toLongOrNull() ?: throw IOException("Valeur encodeur invalide.") }
    }

    @PluginMethod
    fun startSensors(call: PluginCall) {
        val manager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
        val sensor = manager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR)
        if (sensor == null) {
            call.reject("Ce téléphone ne fournit pas de capteur de rotation.")
            return
        }

        stopSensorUpdates()
        val intervalMs = (call.getInt("intervalMs") ?: 10).coerceIn(10, 100)
        val thread = HandlerThread("DobsonSensors").apply { start() }
        sensorManager = manager
        sensorThread = thread
        sensorHandler = Handler(thread.looper)
        if (!manager.registerListener(this, sensor, intervalMs * 1000, sensorHandler)) {
            stopSensorUpdates()
            call.reject("Android n'a pas pu démarrer le capteur de rotation.")
            return
        }
        call.resolve()
    }

    @PluginMethod
    fun stopSensors(call: PluginCall) {
        stopSensorUpdates()
        call.resolve()
    }

    @PluginMethod
    fun saveAstrometryApiKey(call: PluginCall) {
        val apiKey = call.getString("apiKey")?.trim().orEmpty()
        if (apiKey.isBlank()) {
            call.reject("La clé API Astrometry.net est vide.")
            return
        }
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            cipher.init(Cipher.ENCRYPT_MODE, astrometrySecretKey())
            val encrypted = cipher.doFinal(apiKey.toByteArray(Charsets.UTF_8))
            val payload = cipher.iv + encrypted
            context.getSharedPreferences("dobson-secure", Context.MODE_PRIVATE)
                .edit()
                .putString("astrometry-api-key", Base64.encodeToString(payload, Base64.NO_WRAP))
                .apply()
            call.resolve()
        } catch (error: Exception) {
            call.reject("Impossible de chiffrer la clé API Astrometry.net.", error)
        }
    }

    @PluginMethod
    fun getAstrometryApiKey(call: PluginCall) {
        try {
            val encoded = context.getSharedPreferences("dobson-secure", Context.MODE_PRIVATE)
                .getString("astrometry-api-key", null)
            if (encoded == null) {
                call.resolve(JSObject().put("apiKey", ""))
                return
            }
            val payload = Base64.decode(encoded, Base64.NO_WRAP)
            if (payload.size <= 12) throw IllegalStateException("La clé API chiffrée est invalide.")
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            cipher.init(
                Cipher.DECRYPT_MODE,
                astrometrySecretKey(),
                GCMParameterSpec(128, payload.copyOfRange(0, 12))
            )
            val apiKey = cipher.doFinal(payload.copyOfRange(12, payload.size)).toString(Charsets.UTF_8)
            call.resolve(JSObject().put("apiKey", apiKey))
        } catch (error: Exception) {
            call.reject("Impossible de déchiffrer la clé API Astrometry.net.", error)
        }
    }

    @PluginMethod
    fun clearAstrometryApiKey(call: PluginCall) {
        context.getSharedPreferences("dobson-secure", Context.MODE_PRIVATE)
            .edit()
            .remove("astrometry-api-key")
            .apply()
        call.resolve()
    }

    override fun onSensorChanged(event: SensorEvent) {
        if (event.sensor.type != Sensor.TYPE_ROTATION_VECTOR) return
        val rotation = FloatArray(9)
        val orientation = FloatArray(3)
        SensorManager.getRotationMatrixFromVector(rotation, event.values)
        SensorManager.getOrientation(rotation, orientation)

        val east = rotation[1].toDouble()
        val north = rotation[4].toDouble()
        val up = rotation[7].toDouble()
        val altitude = Math.toDegrees(kotlin.math.asin(up.coerceIn(-1.0, 1.0)))
        val azimuth = ((Math.toDegrees(kotlin.math.atan2(east, north)) % 360.0) + 360.0) % 360.0
        val cameraEast = -rotation[2].toDouble()
        val cameraNorth = -rotation[5].toDouble()
        val cameraUp = -rotation[8].toDouble()
        val cameraAltitude = Math.toDegrees(kotlin.math.asin(cameraUp.coerceIn(-1.0, 1.0)))
        val cameraAzimuth = ((Math.toDegrees(kotlin.math.atan2(cameraEast, cameraNorth)) % 360.0) + 360.0) % 360.0
        val payload = JSObject()
            .put("azimuth", azimuth)
            .put("altitude", altitude)
            .put("cameraAzimuth", cameraAzimuth)
            .put("cameraAltitude", cameraAltitude)
            .put("pitch", Math.toDegrees(orientation[1].toDouble()))
            .put("roll", Math.toDegrees(orientation[2].toDouble()))
            .put("timestamp", event.timestamp / 1_000_000.0)
            .put("headingAvailable", true)
        notifyListeners("orientation", payload)
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) = Unit

    @PluginMethod
    fun initializeCatalog(call: PluginCall) {
        val revision = call.getString("revision")
        val rawCatalog = call.getString("catalogJson")
        if (revision.isNullOrBlank() || rawCatalog.isNullOrBlank()) {
            call.reject("Le catalogue ou sa révision est absent.")
            return
        }

        try {
            val helper = catalogDatabase()
            helper.writableDatabase.use { db ->
                val currentRevision = db.rawQuery(
                    "SELECT value FROM catalog_meta WHERE key = 'revision'",
                    null
                ).use { cursor -> if (cursor.moveToFirst()) cursor.getString(0) else null }
                if (currentRevision != revision) {
                    val objects = JSONArray(rawCatalog)
                    db.beginTransaction()
                    try {
                        db.delete("objects", null, null)
                        for (index in 0 until objects.length()) {
                            val item = objects.getJSONObject(index)
                            db.execSQL(
                                "INSERT INTO objects (name, common_name, type, ra, dec, mag) VALUES (?, ?, ?, ?, ?, ?)",
                                arrayOf(
                                    item.getString("name"),
                                    item.optString("commonName"),
                                    item.optString("type"),
                                    item.getDouble("ra"),
                                    item.getDouble("dec"),
                                    item.getDouble("mag")
                                )
                            )
                        }
                        db.execSQL(
                            "INSERT OR REPLACE INTO catalog_meta (key, value) VALUES ('revision', ?)",
                            arrayOf(revision)
                        )
                        db.setTransactionSuccessful()
                    } finally {
                        db.endTransaction()
                    }
                }
            }
            call.resolve()
        } catch (error: Exception) {
            call.reject("Impossible d'enregistrer le catalogue dans SQLite.", error)
        }
    }

    @PluginMethod
    fun getCatalog(call: PluginCall) {
        try {
            call.resolve(JSObject().put("objects", queryCatalog(null, 0)))
        } catch (error: Exception) {
            call.reject("Impossible de lire le catalogue SQLite.", error)
        }
    }

    @PluginMethod
    fun searchCatalog(call: PluginCall) {
        val query = call.getString("query")?.trim().orEmpty()
        val limit = (call.getInt("limit") ?: 100).coerceIn(1, 1000)
        try {
            call.resolve(JSObject().put("objects", queryCatalog(query, limit)))
        } catch (error: Exception) {
            call.reject("La recherche dans le catalogue SQLite a échoué.", error)
        }
    }

    @PluginMethod
    fun zwoListCameras(call: PluginCall) {
        zwoExecutor.execute {
            try {
                ASIUSBManager.initContext(context)
                val cameras = JSArray()
                val count = ZwoCamera.getNumOfConnectedCameras()
                for (index in 0 until count) {
                    val property = ZwoCamera.getCameraProperty(index).obj as? ASICameraProperty ?: continue
                    cameras.put(
                        JSObject()
                            .put("index", index)
                            .put("cameraId", property.cameraID)
                            .put("name", property.name)
                            .put("maxWidth", property.maxWidth)
                            .put("maxHeight", property.maxHeight)
                            .put("isColor", property.isColorCam == 1)
                    )
                }
                call.resolve(JSObject().put("cameras", cameras))
            } catch (error: Throwable) {
                zwoLog("Échec de la liste des caméras.", error)
                rejectZwo(call, "Impossible de lister les caméras ZWO.", error)
            }
        }
    }

    @PluginMethod
    fun zwoGetLog(call: PluginCall) {
        zwoExecutor.execute {
            val content = try {
                zwoLogFile().takeIf { it.exists() }?.readText() ?: ""
            } catch (error: Throwable) {
                "Lecture du journal impossible : ${error.message}"
            }
            call.resolve(JSObject().put("content", content))
        }
    }

    @PluginMethod
    fun zwoClearLog(call: PluginCall) {
        zwoExecutor.execute {
            zwoLogFile().delete()
            call.resolve()
        }
    }

    @PluginMethod
    fun zwoStatus(call: PluginCall) {
        zwoExecutor.execute {
            call.resolve(zwoStatusObject())
        }
    }

    @PluginMethod
    fun zwoOpen(call: PluginCall) {
        val request = ZwoOpenRequest(
            call,
            call.getInt("index") ?: 0,
            call.getDouble("exposureMs") ?: zwoExposureMs,
            call.getInt("gain") ?: zwoGain
        )
        zwoExecutor.execute {
            try {
                ASIUSBManager.initContext(context)
                val device = findZwoUsbDevice()
                if (device == null) {
                    zwoLog("Aucune caméra ZWO détectée sur le port USB.")
                    call.reject("Aucune caméra ZWO détectée sur le port USB.")
                    return@execute
                }
                val usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
                if (usbManager.hasPermission(device)) {
                    openZwoLocked(request)
                } else {
                    requestZwoPermission(request, device)
                }
            } catch (error: Throwable) {
                zwoLog("Échec de l'ouverture (préparation).", error)
                rejectZwo(call, "Impossible d'ouvrir la caméra ZWO.", error)
            }
        }
    }

    @PluginMethod
    fun zwoSetControls(call: PluginCall) {
        val requestedExposure = call.getDouble("exposureMs")
        val requestedGain = call.getInt("gain")
        zwoExecutor.execute {
            val camera = zwoCamera
            if (camera == null) {
                call.reject("Caméra ZWO non connectée.")
                return@execute
            }
            try {
                val exposure = (requestedExposure ?: zwoExposureMs).coerceIn(0.1, 30000.0)
                val gain = (requestedGain ?: zwoGain).coerceIn(0, 600)
                applyZwoControls(camera, exposure, gain)
                zwoExposureMs = exposure
                zwoGain = gain
                call.resolve(zwoStatusObject())
            } catch (error: Throwable) {
                zwoLog("Échec de la modification des réglages.", error)
                rejectZwo(call, "Impossible de modifier les réglages de la caméra ZWO.", error)
            }
        }
    }

    @PluginMethod
    fun zwoGetFrame(call: PluginCall) {
        zwoExecutor.execute {
            val camera = zwoCamera
            val buffer = zwoBuffer
            if (camera == null || buffer == null) {
                call.reject("Caméra ZWO non connectée.")
                return@execute
            }
            try {
                val timeoutMs = (zwoExposureMs * 2 + 2000).toInt()
                val code = camera.getVideoData(buffer, zwoBufferSize, timeoutMs).intVal
                if (code != zwoSuccess) {
                    call.reject(zwoErrorMessage(code))
                    return@execute
                }
                val raw = ByteArray(zwoBufferSize)
                val byteBuffer = buffer.getmByteBuffer()
                byteBuffer.rewind()
                byteBuffer.get(raw)
                val bitmap = rawToBitmap(raw)
                val output = ByteArrayOutputStream()
                bitmap.compress(Bitmap.CompressFormat.JPEG, 85, output)
                val width = bitmap.width
                val height = bitmap.height
                bitmap.recycle()
                val encoded = Base64.encodeToString(output.toByteArray(), Base64.NO_WRAP)
                call.resolve(
                    JSObject()
                        .put("dataUrl", "data:image/jpeg;base64,$encoded")
                        .put("width", width)
                        .put("height", height)
                        .put("timestamp", System.currentTimeMillis())
                )
            } catch (error: Throwable) {
                zwoLog("Échec de la lecture d'une image.", error)
                rejectZwo(call, "Impossible de lire une image depuis la caméra ZWO.", error)
            }
        }
    }

    @PluginMethod
    fun zwoClose(call: PluginCall) {
        zwoExecutor.execute {
            try {
                closeZwoLocked()
                call.resolve(zwoStatusObject())
            } catch (error: Throwable) {
                zwoLog("Échec de la fermeture.", error)
                rejectZwo(call, "Impossible de fermer la caméra ZWO.", error)
            }
        }
    }

    private fun zwoStatusObject(): JSObject {
        val connected = zwoCamera != null
        return JSObject()
            .put("connected", connected)
            .put("usbDetected", findZwoUsbDevice() != null)
            .put("name", if (connected) zwoCameraName else "")
            .put("width", if (connected) zwoWidth else 0)
            .put("height", if (connected) zwoHeight else 0)
            .put("isColor", connected && zwoIsColor)
            .put("exposureMs", zwoExposureMs)
            .put("gain", zwoGain)
    }

    private fun rejectZwo(call: PluginCall, message: String, error: Throwable) {
        call.reject("$message ${error.javaClass.simpleName} : ${error.message.orEmpty()}")
    }

    private fun zwoLogFile(): File = File(context.filesDir, "zwo-journal.log")

    private fun zwoLog(message: String, error: Throwable? = null) {
        try {
            val file = zwoLogFile()
            if (file.exists() && file.length() > zwoLogMaxBytes) {
                file.delete()
            }
            val timestamp = java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS", java.util.Locale.FRANCE)
                .format(java.util.Date())
            val details = error?.let { "\n" + it.stackTraceToString() }.orEmpty()
            file.appendText("[$timestamp] $message$details\n")
        } catch (ignored: Throwable) {
            // Le journal ne doit jamais bloquer la caméra.
        }
    }

    private fun zwoErrorMessage(code: Int): String {
        return "Erreur caméra ZWO : " + ASIConstants.ASI_ERROR_CODE.getErrorString(code)
    }

    private fun findZwoUsbDevice(): UsbDevice? {
        val usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
        return usbManager.deviceList.values.firstOrNull { it.vendorId == zwoVendorId }
    }

    private fun requestZwoPermission(request: ZwoOpenRequest, device: UsbDevice) {
        zwoPendingOpen = request
        if (zwoPermissionReceiver == null) {
            val receiver = object : BroadcastReceiver() {
                override fun onReceive(receiverContext: Context?, intent: Intent?) {
                    val granted = intent?.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) == true
                    zwoExecutor.execute { onZwoPermissionResult(granted) }
                }
            }
            zwoPermissionReceiver = receiver
            val filter = IntentFilter(zwoPermissionAction)
            if (Build.VERSION.SDK_INT >= 33) {
                context.registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED)
            } else {
                context.registerReceiver(receiver, filter)
            }
        }
        val permissionIntent = Intent(zwoPermissionAction).setPackage(context.packageName)
        val flags = PendingIntent.FLAG_UPDATE_CURRENT or
            (if (Build.VERSION.SDK_INT >= 31) PendingIntent.FLAG_MUTABLE else 0)
        val pendingIntent = PendingIntent.getBroadcast(context, 0, permissionIntent, flags)
        val usbManager = context.getSystemService(Context.USB_SERVICE) as UsbManager
        usbManager.requestPermission(device, pendingIntent)
    }

    private fun onZwoPermissionResult(granted: Boolean) {
        val request = zwoPendingOpen ?: return
        zwoPendingOpen = null
        unregisterZwoPermissionReceiver()
        if (!granted) {
            zwoLog("Permission USB refusée par l'utilisateur.")
            request.call.reject("Permission USB refusée pour la caméra ZWO.")
            return
        }
        openZwoLocked(request)
    }

    private fun unregisterZwoPermissionReceiver() {
        val receiver = zwoPermissionReceiver ?: return
        zwoPermissionReceiver = null
        try {
            context.unregisterReceiver(receiver)
        } catch (ignored: IllegalArgumentException) {
            // Le récepteur était déjà retiré.
        }
    }

    private fun openZwoLocked(request: ZwoOpenRequest) {
        val call = request.call
        var camera: ZwoCamera? = null
        try {
            closeZwoLocked()
            val count = ZwoCamera.getNumOfConnectedCameras()
            if (request.index !in 0 until count) {
                call.reject("Caméra ZWO introuvable.")
                return
            }
            val property = ZwoCamera.getCameraProperty(request.index).obj as? ASICameraProperty
            if (property == null) {
                call.reject("Propriétés de la caméra ZWO illisibles.")
                return
            }
            val opened = ZwoCamera(property.cameraID)
            camera = opened
            val openCode = opened.openCamera().intVal
            if (openCode != zwoSuccess) {
                call.reject(zwoErrorMessage(openCode))
                return
            }
            opened.initCamera()
            zwoLog("Caméra ouverte : ${property.name}, initialisation effectuée.")
            val width = property.maxWidth.toInt()
            val height = property.maxHeight.toInt()
            val roiCode = opened.setRoiFormat(width, height, 1, ASIConstants.ASI_IMG_TYPE.ASI_IMG_RAW8).intVal
            if (roiCode != zwoSuccess) {
                zwoLog("Format d'image refusé : ${zwoErrorMessage(roiCode)}")
                call.reject(zwoErrorMessage(roiCode))
                return
            }
            val exposure = request.exposureMs.coerceIn(0.1, 30000.0)
            val gain = request.gain.coerceIn(0, 600)
            applyZwoControls(opened, exposure, gain)
            val startCode = opened.startVideoCapture().intVal
            if (startCode != zwoSuccess) {
                zwoLog("Démarrage du flux refusé : ${zwoErrorMessage(startCode)}")
                call.reject(zwoErrorMessage(startCode))
                return
            }
            val size = width * height
            zwoCameraName = property.name
            zwoWidth = width
            zwoHeight = height
            zwoIsColor = property.isColorCam == 1
            zwoBayerPattern = property.bayerPattern
            zwoBufferSize = size
            zwoBuffer = ASIImageBuffer.allocate(size)
            zwoExposureMs = exposure
            zwoGain = gain
            zwoCamera = opened
            call.resolve(zwoStatusObject())
        } catch (error: Throwable) {
            camera?.let { closeQuietly(it) }
            zwoLog("Échec de l'ouverture de la caméra.", error)
            rejectZwo(call, "Impossible d'ouvrir la caméra ZWO.", error)
        }
    }

    private fun applyZwoControls(camera: ZwoCamera, exposureMs: Double, gain: Int) {
        val gainCode = camera.setControlValue(ASIConstants.ASI_CONTROL_TYPE.ASI_GAIN, gain.toLong(), 0)
        if (gainCode != zwoSuccess) {
            throw IllegalStateException(zwoErrorMessage(gainCode))
        }
        val exposureUs = Math.round(exposureMs * 1000.0)
        val exposureCode = camera.setControlValue(ASIConstants.ASI_CONTROL_TYPE.ASI_EXPOSURE, exposureUs, 0)
        if (exposureCode != zwoSuccess) {
            throw IllegalStateException(zwoErrorMessage(exposureCode))
        }
    }

    private fun closeZwoLocked() {
        val camera = zwoCamera ?: return
        zwoCamera = null
        zwoBuffer = null
        zwoBufferSize = 0
        closeQuietly(camera)
    }

    private fun closeQuietly(camera: ZwoCamera) {
        try {
            camera.stopVideoCapture()
        } catch (ignored: Exception) {
            // La caméra peut déjà être arrêtée.
        }
        try {
            camera.closeCamera()
        } catch (ignored: Exception) {
            // La caméra peut déjà être fermée.
        }
    }

    private fun rawToBitmap(raw: ByteArray): Bitmap {
        val histogram = IntArray(256)
        for (value in raw) {
            histogram[value.toInt() and 0xFF]++
        }
        val low = histogramPercentile(histogram, raw.size, 0.005)
        val high = histogramPercentile(histogram, raw.size, 0.998).coerceAtLeast(low + 1)
        val lut = IntArray(256) { value -> ((value - low) * 255 / (high - low)).coerceIn(0, 255) }
        return if (zwoIsColor) rawColorToBitmap(raw, lut) else rawMonoToBitmap(raw, lut)
    }

    private fun histogramPercentile(histogram: IntArray, total: Int, fraction: Double): Int {
        val target = (total * fraction).toLong()
        var cumulative = 0L
        for (value in histogram.indices) {
            cumulative += histogram[value]
            if (cumulative > target) return value
        }
        return 255
    }

    private fun rawMonoToBitmap(raw: ByteArray, lut: IntArray): Bitmap {
        val pixels = IntArray(zwoWidth * zwoHeight)
        for (index in pixels.indices) {
            val gray = lut[raw[index].toInt() and 0xFF]
            pixels[index] = Color.rgb(gray, gray, gray)
        }
        return Bitmap.createBitmap(pixels, zwoWidth, zwoHeight, Bitmap.Config.ARGB_8888)
    }

    private fun rawColorToBitmap(raw: ByteArray, lut: IntArray): Bitmap {
        val layout = when (zwoBayerPattern) {
            ASIConstants.ASI_BAYER_PATTERN.ASI_BAYER_BG -> "BGGR"
            ASIConstants.ASI_BAYER_PATTERN.ASI_BAYER_GR -> "GRBG"
            ASIConstants.ASI_BAYER_PATTERN.ASI_BAYER_GB -> "GBRG"
            else -> "RGGB"
        }
        val redOffset = layout.indexOf('R')
        val blueOffset = layout.indexOf('B')
        val greenOffsets = layout.indices.filter { layout[it] == 'G' }
        val offsets = intArrayOf(0, 1, zwoWidth, zwoWidth + 1)
        val outWidth = zwoWidth / 2
        val outHeight = zwoHeight / 2
        val pixels = IntArray(outWidth * outHeight)
        for (y in 0 until outHeight) {
            for (x in 0 until outWidth) {
                val base = 2 * y * zwoWidth + 2 * x
                val red = lut[raw[base + offsets[redOffset]].toInt() and 0xFF]
                val green = (
                    lut[raw[base + offsets[greenOffsets[0]]].toInt() and 0xFF] +
                        lut[raw[base + offsets[greenOffsets[1]]].toInt() and 0xFF]
                    ) / 2
                val blue = lut[raw[base + offsets[blueOffset]].toInt() and 0xFF]
                pixels[y * outWidth + x] = Color.rgb(red, green, blue)
            }
        }
        return Bitmap.createBitmap(pixels, outWidth, outHeight, Bitmap.Config.ARGB_8888)
    }

    override fun handleOnDestroy() {
        stopSensorUpdates()
        unregisterZwoPermissionReceiver()
        zwoPendingOpen = null
        zwoExecutor.execute { closeZwoLocked() }
        zwoExecutor.shutdown()
        val connectivity = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        releaseNetworkRequest(connectivity)
        encoderExecutor.shutdownNow()
        database?.close()
        database = null
        super.handleOnDestroy()
    }

    private fun releaseNetworkRequest(connectivity: ConnectivityManager) {
        networkCallback?.let {
            try {
                connectivity.bindProcessToNetwork(null)
                connectivity.unregisterNetworkCallback(it)
            } catch (_: IllegalArgumentException) {
                // The callback may already have been removed by Android after network loss.
            }
        }
        networkCallback = null
        pendingWifiCall?.reject("La demande de connexion Wi-Fi a été annulée.")
        pendingWifiCall = null
    }

    private fun stopSensorUpdates() {
        sensorManager?.unregisterListener(this)
        sensorThread?.quitSafely()
        sensorManager = null
        sensorHandler = null
        sensorThread = null
    }

    private fun catalogDatabase(): CatalogDatabase {
        return database ?: CatalogDatabase(context).also { database = it }
    }

    private fun astrometrySecretKey(): SecretKey {
        val keyStore = KeyStore.getInstance("AndroidKeyStore").apply { load(null) }
        val alias = "dobson-astrometry-api-key"
        val existing = keyStore.getKey(alias, null) as? SecretKey
        if (existing != null) return existing
        val generator = KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore")
        generator.init(
            KeyGenParameterSpec.Builder(
                alias,
                KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT
            )
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setRandomizedEncryptionRequired(true)
                .build()
        )
        return generator.generateKey()
    }

    private fun queryCatalog(query: String?, limit: Int): JSArray {
        val db: SQLiteDatabase = catalogDatabase().readableDatabase
        val sql = if (query.isNullOrBlank()) {
            "SELECT name, common_name, type, ra, dec, mag FROM objects ORDER BY name"
        } else {
            "SELECT name, common_name, type, ra, dec, mag FROM objects WHERE name LIKE ? OR common_name LIKE ? OR type LIKE ? ORDER BY name LIMIT ?"
        }
        val args = if (query.isNullOrBlank()) null else arrayOf("%$query%", "%$query%", "%$query%", limit.toString())
        val result = JSONArray()
        db.rawQuery(sql, args).use { cursor ->
            while (cursor.moveToNext()) {
                result.put(
                    JSONObject()
                        .put("name", cursor.getString(0))
                        .put("commonName", cursor.getString(1))
                        .put("type", cursor.getString(2))
                        .put("ra", cursor.getDouble(3))
                        .put("dec", cursor.getDouble(4))
                        .put("mag", cursor.getDouble(5))
                )
            }
        }
        return JSArray(result)
    }

    private class CatalogDatabase(context: Context) : SQLiteOpenHelper(context, "dobson-catalog.db", null, 1) {
        override fun onCreate(db: SQLiteDatabase) {
            db.execSQL(
                "CREATE TABLE objects (name TEXT NOT NULL, common_name TEXT NOT NULL, type TEXT NOT NULL, ra REAL NOT NULL, dec REAL NOT NULL, mag REAL NOT NULL)"
            )
            db.execSQL("CREATE INDEX objects_name_idx ON objects(name COLLATE NOCASE)")
            db.execSQL("CREATE INDEX objects_type_idx ON objects(type COLLATE NOCASE)")
            db.execSQL("CREATE TABLE catalog_meta (key TEXT PRIMARY KEY, value TEXT NOT NULL)")
        }

        override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) = Unit
    }
}
