package fr.dobsonpushto.mobile

import org.json.JSONArray
import org.json.JSONObject
import java.io.BufferedReader
import java.io.File
import java.io.InputStream
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.URL
import java.net.URLEncoder
import java.nio.charset.StandardCharsets

class AstrometryOnlineClient {
    data class Solution(val rightAscensionDegrees: Double, val declinationDegrees: Double)

    fun solve(
        image: File,
        apiKey: String,
        onProgress: (String) -> Unit
    ): Solution {
        require(image.isFile && image.length() > 0L) { "Le fichier image est introuvable ou vide." }
        require(apiKey.isNotBlank()) { "Saisis une clé API Astrometry.net." }
        checkInterrupted()

        onProgress("Connexion à Astrometry.net…")
        val login = postJson("login", JSONObject().put("apikey", apiKey))
        requireSuccess(login, "Connexion refusée")
        val session = login.optString("session")
        check(session.isNotBlank()) { "Astrometry.net n'a pas renvoyé de session valide." }

        onProgress("Envoi de l'image à Astrometry.net…")
        val upload = uploadImage(image, session)
        requireSuccess(upload, "Envoi refusé")
        val submissionId = upload.optLong("subid", -1L)
        check(submissionId > 0L) { "Astrometry.net n'a pas renvoyé de numéro de soumission." }

        val deadline = System.currentTimeMillis() + SOLVE_TIMEOUT_MS
        var jobId: Long? = null
        while (System.currentTimeMillis() < deadline) {
            checkInterrupted()
            if (jobId == null) {
                val submission = getJson("submissions/$submissionId")
                val jobs = submission.optJSONArray("jobs")
                jobId = jobs?.lastNonEmptyLong()
                if (jobId == null) {
                    onProgress("Image reçue · en attente d'un serveur de calcul…")
                    sleepBeforePoll()
                    continue
                }
            }

            val status = getJson("jobs/$jobId").optString("status")
            when (status) {
                "success" -> {
                    val calibration = getJson("jobs/$jobId/calibration")
                    val ra = calibration.optDouble("ra", Double.NaN)
                    val dec = calibration.optDouble("dec", Double.NaN)
                    check(ra.isFinite() && ra in 0.0..360.0 && dec.isFinite() && dec in -90.0..90.0) {
                        "Astrometry.net a terminé sans renvoyer de coordonnées valides."
                    }
                    return Solution(ra, dec)
                }
                "failure" -> throw IllegalStateException(
                    "Astrometry.net n'a pas réussi à résoudre cette image. Vérifie le champ et la qualité de la photo."
                )
                "solving", "queued", "processing" ->
                    onProgress("Résolution en ligne en cours… état : $status.")
                else -> onProgress("Soumission reçue · état : ${status.ifBlank { "en attente" }}.")
            }
            sleepBeforePoll()
        }
        throw IllegalStateException("Le service Astrometry.net n'a pas terminé dans le délai imparti.")
    }

    private fun postJson(path: String, body: JSONObject): JSONObject =
        withConnection(path, "POST") { connection ->
            connection.doOutput = true
            connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded; charset=utf-8")
            val formBody = "request-json=" + URLEncoder.encode(body.toString(), StandardCharsets.UTF_8.name())
            connection.outputStream.use { output ->
                output.write(formBody.toByteArray(StandardCharsets.UTF_8))
            }
            readJsonResponse(connection)
        }

    private fun uploadImage(image: File, session: String): JSONObject =
        withConnection("upload", "POST") { connection ->
            val boundary = "DobsonAstrometry${System.nanoTime()}"
            val requestJson = JSONObject()
                .put("session", session)
                .put("publicly_visible", "n")
                .put("allow_commercial_use", "d")
                .put("allow_modifications", "d")
            val fileName = image.name.replace("\"", "_").replace("\r", "_").replace("\n", "_")
            val prefix = buildString {
                append("--$boundary\r\n")
                append("Content-Disposition: form-data; name=\"request-json\"\r\n")
                append("Content-Type: text/plain; charset=utf-8\r\n\r\n")
                append(requestJson)
                append("\r\n--$boundary\r\n")
                append("Content-Disposition: form-data; name=\"file\"; filename=\"$fileName\"\r\n")
                append("Content-Type: application/octet-stream\r\n\r\n")
            }.toByteArray(StandardCharsets.UTF_8)
            val suffix = "\r\n--$boundary--\r\n".toByteArray(StandardCharsets.UTF_8)
            val contentLength = prefix.size.toLong() + image.length() + suffix.size

            connection.setRequestProperty("Content-Type", "multipart/form-data; boundary=$boundary")
            connection.setFixedLengthStreamingMode(contentLength)
            connection.doOutput = true
            connection.outputStream.use { output ->
                output.write(prefix)
                image.inputStream().use { input -> input.copyTo(output, UPLOAD_BUFFER_SIZE) }
                output.write(suffix)
            }
            readJsonResponse(connection)
        }

    private fun getJson(path: String): JSONObject =
        withConnection(path, "GET") { connection -> readJsonResponse(connection) }

    private fun <T> withConnection(path: String, method: String, action: (HttpURLConnection) -> T): T {
        checkInterrupted()
        val connection = (URL("$API_BASE_URL/$path").openConnection() as HttpURLConnection).apply {
            requestMethod = method
            connectTimeout = CONNECT_TIMEOUT_MS
            readTimeout = READ_TIMEOUT_MS
            instanceFollowRedirects = true
            setRequestProperty("Accept", "application/json")
            setRequestProperty("User-Agent", "Dobson-PushTo-Android")
        }
        return try {
            action(connection)
        } finally {
            connection.disconnect()
        }
    }

    private fun readJsonResponse(connection: HttpURLConnection): JSONObject {
        val code = connection.responseCode
        val stream = if (code in 200..299) connection.inputStream else connection.errorStream
        val body = stream?.use(::readLimitedText).orEmpty()
        if (code !in 200..299) {
            throw IllegalStateException("Erreur HTTP $code depuis Astrometry.net${body.takeIf { it.isNotBlank() }?.let { ": $it" } ?: "."}")
        }
        return try {
            JSONObject(body)
        } catch (error: Exception) {
            throw IllegalStateException("Réponse Astrometry.net illisible.", error)
        }
    }

    private fun readLimitedText(stream: InputStream): String =
        BufferedReader(InputStreamReader(stream, StandardCharsets.UTF_8)).use { reader ->
            val result = StringBuilder()
            val buffer = CharArray(4096)
            var count: Int
            while (reader.read(buffer).also { count = it } >= 0) {
                if (count == 0) continue
                if (result.length + count > MAX_RESPONSE_CHARS) {
                    throw IllegalStateException("Réponse trop volumineuse reçue depuis Astrometry.net.")
                }
                result.append(buffer, 0, count)
            }
            result.toString()
        }

    private fun JSONArray.lastNonEmptyLong(): Long? {
        for (index in length() - 1 downTo 0) {
            val value = optLong(index, -1L)
            if (value > 0L) return value
        }
        return null
    }

    private fun requireSuccess(response: JSONObject, context: String) {
        if (response.optString("status") != "success") {
            val detail = response.optString("errormessage").takeIf { it.isNotBlank() }
                ?: response.optString("message").takeIf { it.isNotBlank() }
                ?: "Vérifie ta clé API et réessaie."
            throw IllegalStateException("$context : $detail")
        }
    }

    private fun checkInterrupted() {
        if (Thread.currentThread().isInterrupted) {
            throw InterruptedException("Résolution en ligne interrompue.")
        }
    }

    private fun sleepBeforePoll() {
        checkInterrupted()
        Thread.sleep(POLL_INTERVAL_MS)
    }

    companion object {
        private const val API_BASE_URL = "https://nova.astrometry.net/api"
        private const val CONNECT_TIMEOUT_MS = 20_000
        private const val READ_TIMEOUT_MS = 45_000
        private const val POLL_INTERVAL_MS = 4_000L
        private const val SOLVE_TIMEOUT_MS = 20 * 60 * 1000L
        private const val UPLOAD_BUFFER_SIZE = 64 * 1024
        private const val MAX_RESPONSE_CHARS = 2 * 1024 * 1024
    }
}
