package fr.dobsonpushto.mobile

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Intent
import android.content.pm.ServiceInfo
import android.os.Build
import android.os.IBinder

class SkySafariService : Service() {
    private var server: SkySafariServer? = null

    override fun onCreate() {
        super.onCreate()
        getSharedPreferences("dobson-mobile", MODE_PRIVATE)
            .edit()
            .putBoolean("skySafariRunning", true)
            .apply()
        createNotificationChannel()
        val notification = buildNotification("Préparation du serveur SkySafari…")
        if (Build.VERSION.SDK_INT >= 29) {
            startForeground(
                NOTIFICATION_ID,
                notification,
                ServiceInfo.FOREGROUND_SERVICE_TYPE_CONNECTED_DEVICE
            )
        } else {
            startForeground(NOTIFICATION_ID, notification)
        }
        server = SkySafariServer(TelescopeModel(this)) { status ->
            getSharedPreferences("dobson-mobile", MODE_PRIVATE)
                .edit()
                .putString("skySafariStatus", status)
                .apply()
            val manager = getSystemService(NotificationManager::class.java)
            manager.notify(NOTIFICATION_ID, buildNotification(status))
        }.also { it.start() }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int =
        START_NOT_STICKY

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        server?.close()
        server = null
        getSharedPreferences("dobson-mobile", MODE_PRIVATE)
            .edit()
            .putBoolean("skySafariRunning", false)
            .putString("skySafariStatus", "Serveur SkySafari arrêté.")
            .apply()
        super.onDestroy()
    }

    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) return
        val channel = NotificationChannel(
            CHANNEL_ID,
            "Connexion SkySafari",
            NotificationManager.IMPORTANCE_LOW
        )
        getSystemService(NotificationManager::class.java).createNotificationChannel(channel)
    }

    private fun buildNotification(status: String): Notification {
        val openApp = PendingIntent.getActivity(
            this,
            0,
            Intent(this, MobileMainActivity::class.java),
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )
        val builder = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            Notification.Builder(this, CHANNEL_ID)
        } else {
            @Suppress("DEPRECATION")
            Notification.Builder(this)
        }
        return builder
            .setContentTitle("Dobson Push-To · SkySafari")
            .setContentText(status)
            .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
            .setContentIntent(openApp)
            .setOngoing(true)
            .build()
    }

    companion object {
        const val ACTION_START = "fr.dobsonpushto.mobile.SKYSAFARI_START"
        const val ACTION_STOP = "fr.dobsonpushto.mobile.SKYSAFARI_STOP"
        private const val CHANNEL_ID = "skysafari_tcp"
        private const val NOTIFICATION_ID = 4030
    }
}
