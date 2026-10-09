package fr.dobsonpushto.mobile

import java.io.BufferedInputStream
import java.io.OutputStream
import java.net.Inet4Address
import java.net.NetworkInterface
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.Executors
import java.util.concurrent.atomic.AtomicBoolean

class SkySafariServer(
    private val model: TelescopeModel,
    private val onStatusChanged: (String) -> Unit
) : AutoCloseable {
    private val running = AtomicBoolean(false)
    private val executor = Executors.newCachedThreadPool()
    @Volatile private var serverSocket: ServerSocket? = null
    @Volatile private var connectedClients = 0

    fun start() {
        if (!running.compareAndSet(false, true)) return
        executor.execute {
            try {
                ServerSocket(PORT).use { socket ->
                    serverSocket = socket
                    socket.reuseAddress = true
                    notifyStatus()
                    while (running.get()) {
                        val client = socket.accept()
                        executor.execute { handleClient(client) }
                    }
                }
            } catch (error: Exception) {
                if (running.get()) {
                    running.set(false)
                    onStatusChanged("Serveur SkySafari indisponible : ${error.message ?: "erreur réseau"}.")
                }
            } finally {
                serverSocket = null
            }
        }
    }

    fun isRunning(): Boolean = running.get()

    fun addressSummary(): String {
        val addresses = try {
            NetworkInterface.getNetworkInterfaces().toList()
                .filter { it.isUp && !it.isLoopback }
                .flatMap { it.inetAddresses.toList() }
                .filterIsInstance<Inet4Address>()
                .filterNot { it.isLoopbackAddress }
                .map { it.hostAddress }
                .distinct()
        } catch (error: Exception) {
            emptyList()
        }
        return if (addresses.isEmpty()) {
            "Adresse Wi-Fi indisponible ; port $PORT"
        } else {
            "Connecte SkySafari à l'adresse ${addresses.joinToString(" / ")}:$PORT"
        }
    }

    override fun close() {
        if (!running.compareAndSet(true, false)) return
        try {
            serverSocket?.close()
        } catch (_: Exception) {
        }
        executor.shutdownNow()
    }

    private fun handleClient(client: Socket) {
        connectedClients += 1
        notifyStatus()
        try {
            client.tcpNoDelay = true
            val input = BufferedInputStream(client.getInputStream())
            val output = client.getOutputStream()
            while (running.get() && !client.isClosed) {
                val command = input.read()
                if (command < 0) break
                if (command == '\r'.code || command == '\n'.code) continue
                output.write(response(command))
                output.flush()
            }
        } catch (error: Exception) {
            if (running.get()) onStatusChanged("Connexion SkySafari fermée : ${error.message ?: "erreur réseau"}.")
        } finally {
            connectedClients = (connectedClients - 1).coerceAtLeast(0)
            try {
                client.close()
            } catch (_: Exception) {
            }
            notifyStatus()
        }
    }

    private fun response(command: Int): ByteArray {
        val (azTicks, altTicks) = model.adjustedTicks()
        val text = when (command.toChar()) {
            'a', 'q' -> "%+06d\t%+06d\r".format(model.ticksPerRevAz.toInt(), model.ticksPerRevAlt.toInt())
            'H' -> "y\r"
            'V' -> "V1.0\r"
            'Q', 'R' -> "%+06d\t%+06d\r".format(azTicks, altTicks)
            else -> "%+06d\t%+06d\r".format(azTicks, altTicks)
        }
        return text.toByteArray(Charsets.US_ASCII)
    }

    private fun notifyStatus() {
        onStatusChanged(
            if (!running.get()) "Serveur SkySafari arrêté."
            else if (connectedClients > 0) "SkySafari connecté · port $PORT."
            else addressSummary()
        )
    }

    companion object {
        const val PORT = 4030
    }
}
