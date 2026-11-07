package fr.dobsonpushto.nativeplugin

import android.content.Context
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
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import android.database.sqlite.SQLiteDatabase
import android.database.sqlite.SQLiteOpenHelper
import com.getcapacitor.JSArray
import com.getcapacitor.JSObject
import com.getcapacitor.Plugin
import com.getcapacitor.PluginCall
import com.getcapacitor.PluginMethod
import com.getcapacitor.annotation.CapacitorPlugin
import org.json.JSONArray
import org.json.JSONObject
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

@CapacitorPlugin(name = "DobsonNative")
class DobsonNativePlugin : Plugin(), SensorEventListener {
    private var networkCallback: ConnectivityManager.NetworkCallback? = null
    private var pendingWifiCall: PluginCall? = null
    private var sensorManager: SensorManager? = null
    private var sensorThread: HandlerThread? = null
    private var sensorHandler: Handler? = null
    private var database: CatalogDatabase? = null

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
                    .put("baseUrl", "http://192.168.4.1")
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

    override fun handleOnDestroy() {
        stopSensorUpdates()
        val connectivity = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
        releaseNetworkRequest(connectivity)
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
