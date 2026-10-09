package fr.dobsonpushto.mobile

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.graphics.Typeface
import android.view.MotionEvent
import android.view.View
import kotlin.math.abs
import kotlin.math.cos
import kotlin.math.sin
import kotlin.math.tan

class SkyDomeView(context: Context) : View(context) {
    var objects: List<SkyObject> = emptyList()
        set(value) {
            field = value
            invalidate()
        }
    var latitude = 45.89
        set(value) {
            field = value
            invalidate()
        }
    var longitude = 6.05
        set(value) {
            field = value
            invalidate()
        }
    var azimuth = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var altitude = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var phoneAzimuth = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var phoneAltitude = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var phoneFieldView = false
        set(value) {
            field = value
            invalidate()
        }
    var phoneSensorActive = false
        set(value) {
            field = value
            invalidate()
        }
    var cameraOverlay = false
        set(value) {
            field = value
            invalidate()
        }
    var nightMode = false
        set(value) {
            field = value
            invalidate()
        }
    var sensorConnected = false
        set(value) {
            field = value
            invalidate()
        }
    var lastTelemetryAt = 0L
        set(value) {
            field = value
            invalidate()
        }
    var selectedObject: SkyObject? = null
    var onObjectSelected: ((SkyObject) -> Unit)? = null

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply { textAlign = Paint.Align.CENTER }
    private val hitRegions = mutableListOf<Pair<RectF, SkyObject>>()
    private val density get() = resources.displayMetrics.density

    init {
        contentDescription = "Champ céleste orienté selon les capteurs du téléphone, avec caméra en réalité augmentée."
        isClickable = true
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (phoneFieldView) {
            drawPhoneSky(canvas)
            return
        }
        val background = if (nightMode) Color.rgb(24, 3, 8) else Color.rgb(8, 18, 34)
        val grid = if (nightMode) Color.rgb(130, 35, 48) else Color.rgb(59, 91, 130)
        val accent = if (nightMode) Color.rgb(255, 93, 105) else Color.rgb(255, 193, 104)
        val labels = if (nightMode) Color.rgb(255, 173, 180) else Color.rgb(206, 224, 250)
        val star = if (nightMode) Color.rgb(255, 186, 190) else Color.rgb(228, 238, 255)
        canvas.drawColor(background)

        val centerX = width * 0.5f
        val centerY = height * 0.49f
        val radius = (minOf(width * 0.43f, height * 0.39f)).coerceAtLeast(1f)
        paint.style = Paint.Style.FILL
        paint.color = if (nightMode) Color.rgb(39, 5, 12) else Color.rgb(14, 29, 49)
        canvas.drawCircle(centerX, centerY, radius, paint)

        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 1f * density
        paint.color = grid
        canvas.drawCircle(centerX, centerY, radius, paint)
        listOf(30.0, 60.0).forEach { elevation ->
            val ringRadius = radius * (90.0 - elevation) / 90.0
            canvas.drawCircle(centerX, centerY, ringRadius.toFloat(), paint)
        }
        for (bearing in 0 until 360 step 30) {
            val radians = Math.toRadians(bearing.toDouble())
            val edgeX = centerX + radius * sin(radians).toFloat()
            val edgeY = centerY - radius * cos(radians).toFloat()
            canvas.drawLine(centerX, centerY, edgeX, edgeY, paint)
        }

        textPaint.color = labels
        textPaint.textSize = 12f * density
        listOf("N" to 0.0, "E" to 90.0, "S" to 180.0, "O" to 270.0).forEach { (label, bearing) ->
            val radians = Math.toRadians(bearing)
            val x = centerX + (radius + 15f * density) * sin(radians).toFloat()
            val y = centerY - (radius + 15f * density) * cos(radians).toFloat() + 4f * density
            canvas.drawText(label, x, y, textPaint)
        }
        textPaint.textSize = 9f * density
        listOf(60.0, 30.0, 0.0).forEach { elevation ->
            val ringRadius = radius * (90.0 - elevation) / 90.0
            canvas.drawText(
                "${elevation.toInt()}°",
                centerX + 20f * density,
                centerY - ringRadius.toFloat() + 4f * density,
                textPaint
            )
        }

        hitRegions.clear()
        val now = System.currentTimeMillis()
        objects.forEach { item ->
            val (ra, dec) = Astronomy.precess(item.ra, item.dec, now)
            val horizontal = Astronomy.toHorizontal(ra, dec, latitude, longitude, now)
            if (horizontal.altitude < 0.0) return@forEach
            val point = project(
                centerX,
                centerY,
                radius,
                horizontal.azimuth,
                horizontal.altitude
            )
            val magnitudeRadius = (5.2 - item.magnitude.coerceIn(-4.0, 10.0) * 0.2)
                .coerceIn(2.5, 7.5) * density
            paint.style = Paint.Style.FILL
            paint.color = if (selectedObject?.name == item.name) accent else star
            canvas.drawCircle(point.first, point.second, magnitudeRadius.toFloat(), paint)
            if (item.magnitude <= 2.5 || selectedObject?.name == item.name) {
                textPaint.color = labels
                textPaint.textSize = 9f * density
                canvas.drawText(
                    item.commonName.ifBlank { item.name },
                    point.first,
                    point.second - 8f * density,
                    textPaint
                )
            }
            val hitRadius = 16f * density
            hitRegions.add(
                RectF(
                    point.first - hitRadius,
                    point.second - hitRadius,
                    point.first + hitRadius,
                    point.second + hitRadius
                ) to item
            )
        }

        val telescopePoint = project(centerX, centerY, radius, azimuth, altitude.coerceIn(0.0, 90.0))
        paint.style = Paint.Style.STROKE
        paint.color = accent
        paint.strokeWidth = 2f * density
        canvas.drawCircle(telescopePoint.first, telescopePoint.second, 10f * density, paint)
        canvas.drawLine(
            telescopePoint.first - 15f * density,
            telescopePoint.second,
            telescopePoint.first + 15f * density,
            telescopePoint.second,
            paint
        )
        canvas.drawLine(
            telescopePoint.first,
            telescopePoint.second - 15f * density,
            telescopePoint.first,
            telescopePoint.second + 15f * density,
            paint
        )
        paint.style = Paint.Style.FILL

        val telemetryFresh = sensorConnected && now - lastTelemetryAt < 2_000L
        textPaint.color = if (telemetryFresh) {
            if (nightMode) Color.rgb(255, 115, 124) else Color.rgb(129, 225, 171)
        } else {
            if (nightMode) Color.rgb(215, 112, 122) else Color.rgb(255, 183, 120)
        }
        textPaint.textSize = 11f * density
        canvas.drawText(
            if (telemetryFresh) "Encodeurs ESP32 · EN DIRECT" else "Encodeurs ESP32 · hors ligne",
            centerX,
            height - 27f * density,
            textPaint
        )
        textPaint.color = labels
        textPaint.textSize = 10f * density
        canvas.drawText(
            "AZ %.1f°  ·  ALT %.1f°".format(azimuth, altitude),
            centerX,
            height - 10f * density,
            textPaint
        )
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (event.action != MotionEvent.ACTION_UP) return true
        val selected = hitRegions.asReversed().firstOrNull { (bounds, _) ->
            bounds.contains(event.x, event.y)
        }?.second ?: return performClick()
        selectedObject = selected
        onObjectSelected?.invoke(selected)
        invalidate()
        performClick()
        return true
    }

    override fun performClick(): Boolean {
        super.performClick()
        return true
    }

    private fun drawPhoneSky(canvas: Canvas) {
        val background = if (nightMode) Color.rgb(18, 2, 7) else Color.rgb(4, 9, 20)
        val grid = if (nightMode) Color.rgb(145, 48, 58) else Color.rgb(93, 126, 169)
        val accent = if (nightMode) Color.rgb(255, 105, 118) else Color.rgb(255, 206, 115)
        val labels = if (nightMode) Color.rgb(255, 203, 205) else Color.rgb(223, 235, 255)
        val star = if (nightMode) Color.rgb(255, 180, 186) else Color.rgb(238, 244, 255)
        if (!cameraOverlay) canvas.drawColor(background)

        val left = width * 0.08f
        val right = width * 0.92f
        val top = height * 0.12f
        val bottom = height * 0.86f
        val centerX = (left + right) * 0.5f
        val centerY = (top + bottom) * 0.5f
        val halfWidth = (right - left) * 0.5f
        val halfHeight = (bottom - top) * 0.5f

        paint.style = Paint.Style.STROKE
        paint.strokeWidth = density
        paint.color = if (cameraOverlay) Color.argb(175, Color.red(grid), Color.green(grid), Color.blue(grid)) else grid
        canvas.drawRoundRect(left, top, right, bottom, 18f * density, 18f * density, paint)
        for (fraction in listOf(-0.5f, 0.0f, 0.5f)) {
            val y = centerY + halfHeight * fraction
            canvas.drawLine(left, y, right, y, paint)
            val x = centerX + halfWidth * fraction
            canvas.drawLine(x, top, x, bottom, paint)
        }

        val horizontalFov = HORIZONTAL_FIELD_OF_VIEW_DEGREES
        val verticalFov = VERTICAL_FIELD_OF_VIEW_DEGREES
        hitRegions.clear()
        val now = System.currentTimeMillis()
        objects.forEach { item ->
            val (ra, dec) = Astronomy.precess(item.ra, item.dec, now)
            val coordinates = Astronomy.toHorizontal(ra, dec, latitude, longitude, now)
            val deltaAz = Astronomy.angularDifference(coordinates.azimuth, phoneAzimuth)
            val deltaAlt = coordinates.altitude - phoneAltitude
            if (coordinates.altitude < 0.0 ||
                abs(deltaAz) >= horizontalFov * 0.5 ||
                abs(deltaAlt) >= verticalFov * 0.5
            ) return@forEach

            val projectedX = tan(Math.toRadians(deltaAz)) / tan(Math.toRadians(horizontalFov * 0.5))
            val projectedY = tan(Math.toRadians(deltaAlt)) / tan(Math.toRadians(verticalFov * 0.5))
            val x = centerX + halfWidth * projectedX.toFloat()
            val y = centerY - halfHeight * projectedY.toFloat()
            val isSelected = selectedObject?.name == item.name
            val radius = (5.0 - item.magnitude.coerceIn(-4.0, 10.0) * 0.2)
                .coerceIn(2.5, 7.0) * density
            paint.style = Paint.Style.FILL
            paint.color = if (isSelected) accent else star
            canvas.drawCircle(x, y, radius.toFloat(), paint)
            if (item.magnitude <= 3.0 || isSelected) {
                textPaint.color = labels
                textPaint.textSize = 10f * density
                textPaint.typeface = if (isSelected) Typeface.DEFAULT_BOLD else Typeface.DEFAULT
                canvas.drawText(item.commonName.ifBlank { item.name }, x, y - 10f * density, textPaint)
            }
            val hitRadius = 18f * density
            hitRegions.add(RectF(x - hitRadius, y - hitRadius, x + hitRadius, y + hitRadius) to item)
        }

        drawReticle(canvas, centerX, centerY, accent, 16f)
        val telescopeXy = projectInPhoneField(
            centerX,
            centerY,
            halfWidth,
            halfHeight,
            azimuth,
            altitude
        )
        if (telescopeXy != null) drawReticle(canvas, telescopeXy.first, telescopeXy.second, Color.rgb(100, 230, 165), 11f)

        textPaint.color = labels
        textPaint.textSize = 12f * density
        textPaint.typeface = Typeface.DEFAULT
        val direction = if (phoneSensorActive) "Capteurs du téléphone" else "Orientation du téléphone"
        canvas.drawText(
            "AZ ${phoneAzimuth.toInt()}°  ·  ALT ${phoneAltitude.toInt()}°  ·  champ ${horizontalFov.toInt()}°",
            centerX,
            25f * density,
            textPaint
        )
        textPaint.color = if (phoneSensorActive) accent else Color.rgb(255, 175, 95)
        textPaint.textSize = 11f * density
        canvas.drawText(
            if (sensorConnected && now - lastTelemetryAt < 2_000L) {
                "$direction · ESP32 connecté"
            } else {
                "$direction · encodeurs ESP32 hors ligne"
            },
            centerX,
            height - 14f * density,
            textPaint
        )
    }

    private fun drawReticle(canvas: Canvas, x: Float, y: Float, color: Int, sizeDp: Float) {
        paint.style = Paint.Style.STROKE
        paint.color = color
        paint.strokeWidth = 2f * density
        val size = sizeDp * density
        canvas.drawCircle(x, y, size, paint)
        canvas.drawLine(x - size * 1.7f, y, x + size * 1.7f, y, paint)
        canvas.drawLine(x, y - size * 1.7f, x, y + size * 1.7f, paint)
    }

    private fun projectInPhoneField(
        centerX: Float,
        centerY: Float,
        halfWidth: Float,
        halfHeight: Float,
        objectAzimuth: Double,
        objectAltitude: Double
    ): Pair<Float, Float>? {
        val deltaAz = Astronomy.angularDifference(objectAzimuth, phoneAzimuth)
        val deltaAlt = objectAltitude - phoneAltitude
        if (abs(deltaAz) >= HORIZONTAL_FIELD_OF_VIEW_DEGREES * 0.5 ||
            abs(deltaAlt) >= VERTICAL_FIELD_OF_VIEW_DEGREES * 0.5
        ) return null
        val x = centerX + halfWidth *
            (tan(Math.toRadians(deltaAz)) / tan(Math.toRadians(HORIZONTAL_FIELD_OF_VIEW_DEGREES * 0.5))).toFloat()
        val y = centerY - halfHeight *
            (tan(Math.toRadians(deltaAlt)) / tan(Math.toRadians(VERTICAL_FIELD_OF_VIEW_DEGREES * 0.5))).toFloat()
        return x to y
    }

    private fun project(
        centerX: Float,
        centerY: Float,
        radius: Float,
        objectAzimuth: Double,
        objectAltitude: Double
    ): Pair<Float, Float> {
        val radialDistance =
            (radius * (90.0 - objectAltitude.coerceIn(0.0, 90.0)) / 90.0).toFloat()
        val azimuthRadians = Math.toRadians(objectAzimuth)
        return (centerX + radialDistance * sin(azimuthRadians).toFloat()) to
            (centerY - radialDistance * cos(azimuthRadians).toFloat())
    }

    companion object {
        private const val HORIZONTAL_FIELD_OF_VIEW_DEGREES = 90.0
        private const val VERTICAL_FIELD_OF_VIEW_DEGREES = 62.0
    }
}
