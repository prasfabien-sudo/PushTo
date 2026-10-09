package fr.dobsonpushto.mobile

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.graphics.RadialGradient
import android.graphics.RectF
import android.graphics.Shader
import android.graphics.Typeface
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View
import android.view.ViewConfiguration
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
    var objectTypeFilter = "Tous les objets"
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
    var onVisibleObjectsChanged: ((List<SkyObject>) -> Unit)? = null
    var selectedObject: SkyObject? = null
    var onObjectSelected: ((SkyObject) -> Unit)? = null

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val textPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply { textAlign = Paint.Align.CENTER }
    private val hitRegions = mutableListOf<Pair<RectF, SkyObject>>()
    private val density get() = resources.displayMetrics.density
    private val touchSlop = ViewConfiguration.get(context).scaledTouchSlop
    private var fieldOfViewDegrees = DEFAULT_FIELD_OF_VIEW_DEGREES
    private var dragAzimuth = 0.0
    private var dragAltitude = 0.0
    private var downX = 0f
    private var downY = 0f
    private var lastX = 0f
    private var lastY = 0f
    private var dragging = false
    private var scaling = false
    private var lastVisibleObjects: List<SkyObject>? = null
    private val scaleDetector = ScaleGestureDetector(context,
        object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
            override fun onScale(detector: ScaleGestureDetector): Boolean {
                fieldOfViewDegrees = (fieldOfViewDegrees / detector.scaleFactor)
                    .coerceIn(MIN_FIELD_OF_VIEW_DEGREES, MAX_FIELD_OF_VIEW_DEGREES)
                scaling = true
                invalidate()
                return true
            }
        })

    private data class FieldProjection(
        val centerX: Float,
        val centerY: Float,
        val halfWidth: Float,
        val halfHeight: Float,
        val aspect: Float,
        val tanHalfFov: Float,
        val forwardX: Double,
        val forwardY: Double,
        val forwardZ: Double,
        val rightX: Double,
        val rightZ: Double,
        val upX: Double,
        val upY: Double,
        val upZ: Double
    )

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
        scaleDetector.onTouchEvent(event)
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                downX = event.x
                downY = event.y
                lastX = event.x
                lastY = event.y
                dragging = false
                scaling = false
            }
            MotionEvent.ACTION_MOVE -> {
                if (scaleDetector.isInProgress || event.pointerCount > 1) {
                    scaling = true
                    return true
                }
                val totalX = event.x - downX
                val totalY = event.y - downY
                if (!dragging && totalX * totalX + totalY * totalY > touchSlop * touchSlop) {
                    dragging = true
                }
                if (dragging) {
                    dragAzimuth = Astronomy.positiveModulo(
                        dragAzimuth - (event.x - lastX).toDouble() / width.coerceAtLeast(1) *
                            fieldOfViewDegrees * 2.0,
                        360.0
                    )
                    dragAltitude = (dragAltitude + (event.y - lastY).toDouble() / height.coerceAtLeast(1) *
                        fieldOfViewDegrees * 2.0).coerceIn(-80.0, 80.0)
                    invalidate()
                }
                lastX = event.x
                lastY = event.y
            }
            MotionEvent.ACTION_UP -> {
                if (!dragging && !scaling) {
                    hitRegions.asReversed().firstOrNull { (bounds, _) ->
                        bounds.contains(event.x, event.y)
                    }?.second?.let { selected ->
                        selectedObject = selected
                        onObjectSelected?.invoke(selected)
                        invalidate()
                    }
                    performClick()
                }
            }
            MotionEvent.ACTION_CANCEL -> {
                dragging = false
                scaling = false
            }
        }
        return true
    }

    override fun performClick(): Boolean {
        super.performClick()
        return true
    }

    fun recenterPhoneView() {
        dragAzimuth = 0.0
        dragAltitude = 0.0
        fieldOfViewDegrees = DEFAULT_FIELD_OF_VIEW_DEGREES
        invalidate()
    }

    private fun drawPhoneSky(canvas: Canvas) {
        val background = if (nightMode) Color.rgb(18, 2, 7) else Color.rgb(5, 8, 14)
        val grid = if (nightMode) Color.rgb(130, 35, 48) else Color.rgb(59, 79, 105)
        val accent = if (nightMode) Color.rgb(255, 105, 118) else Color.rgb(255, 193, 104)
        val labels = if (nightMode) Color.rgb(255, 203, 205) else Color.rgb(220, 228, 240)
        val left = width * 0.025f
        val right = width * 0.975f
        val top = height * 0.025f
        val bottom = height * 0.975f
        val centerX = (left + right) * 0.5f
        val centerY = (top + bottom) * 0.5f
        val halfWidth = (right - left) * 0.5f
        val halfHeight = (bottom - top) * 0.5f
        val scene = RectF(left, top, right, bottom)
        if (!cameraOverlay) {
            paint.shader = RadialGradient(
                centerX,
                bottom,
                height * 1.15f,
                intArrayOf(Color.rgb(24, 35, 50), Color.rgb(12, 18, 28), background),
                null,
                Shader.TileMode.CLAMP
            )
            canvas.drawRoundRect(scene, 14f * density, 14f * density, paint)
            paint.shader = null
        } else {
            paint.style = Paint.Style.FILL
            paint.color = Color.argb(48, 0, 0, 0)
            canvas.drawRoundRect(scene, 14f * density, 14f * density, paint)
        }
        val clipSave = canvas.save()
        canvas.clipRect(scene)

        val lookAzimuth = Astronomy.positiveModulo(phoneAzimuth + dragAzimuth, 360.0)
        val lookAltitude = (phoneAltitude + dragAltitude).coerceIn(-80.0, 88.0)
        val tanHalfFov = tan(Math.toRadians(fieldOfViewDegrees)).toFloat()
        val azimuthRadians = Math.toRadians(lookAzimuth)
        val altitudeRadians = Math.toRadians(lookAltitude)
        val sinAzimuth = sin(azimuthRadians)
        val cosAzimuth = cos(azimuthRadians)
        val sinAltitude = sin(altitudeRadians)
        val cosAltitude = cos(altitudeRadians)
        val projection = FieldProjection(
            centerX = centerX,
            centerY = centerY,
            halfWidth = halfWidth,
            halfHeight = halfHeight,
            aspect = width.toFloat() / height.coerceAtLeast(1),
            tanHalfFov = tanHalfFov,
            forwardX = sinAzimuth * cosAltitude,
            forwardY = sinAltitude,
            forwardZ = cosAzimuth * cosAltitude,
            rightX = cosAzimuth,
            rightZ = -sinAzimuth,
            upX = -sinAzimuth * sinAltitude,
            upY = cosAltitude,
            upZ = -cosAzimuth * sinAltitude
        )

        hitRegions.clear()
        val now = System.currentTimeMillis()
        val visibleObjects = mutableListOf<Triple<SkyObject, Pair<Float, Float>, Double>>()
        objects.forEach { item ->
            if (!matchesObjectFilter(item)) return@forEach
            val (ra, dec) = Astronomy.precess(item.ra, item.dec, now)
            val coordinates = Astronomy.toHorizontal(ra, dec, latitude, longitude, now)
            if (coordinates.altitude < 0.0) return@forEach
            val point = projectToPhoneField(projection, coordinates.azimuth, coordinates.altitude)
                ?: return@forEach
            if (point.first !in left..right || point.second !in top..bottom) return@forEach
            visibleObjects.add(Triple(item, point, coordinates.altitude))
        }
        val visible = visibleObjects.map { it.first }.take(MAX_VISIBLE_OBJECTS)
        if (visible != lastVisibleObjects) {
            lastVisibleObjects = visible
            onVisibleObjectsChanged?.invoke(visible)
        }

        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 1f * density
        paint.color = Color.argb(if (cameraOverlay) 170 else 210, Color.red(grid), Color.green(grid), Color.blue(grid))
        drawGridLine(canvas, projection, 0.0, 360.0, 2.0, altitudeLine = true)
        listOf(30.0, 60.0).forEach { elevation ->
            drawGridLine(canvas, projection, elevation, 360.0, 2.0, altitudeLine = true)
        }
        for (bearing in 0 until 360 step 45) {
            drawGridLine(canvas, projection, bearing.toDouble(), 88.0, 2.0, altitudeLine = false)
        }

        val labelBackground = if (nightMode) Color.argb(210, 36, 8, 15) else Color.argb(210, 5, 8, 14)
        val placedLabels = mutableListOf<RectF>()
        visibleObjects.sortedBy { it.first.magnitude }.forEach { (item, point, _) ->
            val typeColor = objectColor(item.type)
            val isSelected = selectedObject?.name == item.name
            val color = if (isSelected) accent else typeColor
            val radius = (4.2 - item.magnitude.coerceIn(-4.0, 10.0) * 0.22)
                .coerceIn(2.2, 6.0).toFloat() * density
            paint.style = Paint.Style.FILL
            paint.color = Color.argb(46, Color.red(color), Color.green(color), Color.blue(color))
            canvas.drawCircle(point.first, point.second, radius * 2.7f, paint)
            paint.color = color
            canvas.drawCircle(point.first, point.second, radius, paint)
            if (item.magnitude <= 8.0 || isSelected) {
                val label = item.commonName.ifBlank { item.name }
                textPaint.textSize = 9f * density
                textPaint.typeface = if (isSelected) Typeface.DEFAULT_BOLD else Typeface.DEFAULT
                val labelWidth = textPaint.measureText(label)
                val labelRect = RectF(
                    point.first - labelWidth * 0.5f - 4f * density,
                    point.second - 22f * density,
                    point.first + labelWidth * 0.5f + 4f * density,
                    point.second - 7f * density
                )
                val isInsideScene = labelRect.left >= left + 4f * density &&
                    labelRect.right <= right - 4f * density &&
                    labelRect.top >= top + 24f * density
                if (isInsideScene && placedLabels.none { RectF.intersects(it, labelRect) }) {
                    paint.color = labelBackground
                    canvas.drawRoundRect(labelRect, 4f * density, 4f * density, paint)
                    textPaint.color = if (isSelected) accent else labels
                    canvas.drawText(label, point.first, point.second - 11f * density, textPaint)
                    placedLabels.add(labelRect)
                }
            }
            val hitRadius = 18f * density
            hitRegions.add(
                RectF(
                    point.first - hitRadius,
                    point.second - hitRadius,
                    point.first + hitRadius,
                    point.second + hitRadius
                ) to item
            )
        }

        drawReticle(canvas, centerX, centerY, accent, 9f)
        val telescopePoint = projectToPhoneField(projection, azimuth, altitude)
        if (telescopePoint != null && telescopePoint.first in left..right && telescopePoint.second in top..bottom) {
            drawReticle(canvas, telescopePoint.first, telescopePoint.second, Color.rgb(100, 230, 165), 8f)
        }

        val telemetryFresh = sensorConnected && now - lastTelemetryAt < 2_000L
        val directionLabel = if (phoneSensorActive) "Capteurs" else "Orientation"
        drawHudLabel(
            canvas,
            "$directionLabel AZ %.0f° · ALT %.0f°".format(lookAzimuth, lookAltitude),
            left + 8f * density,
            top + 8f * density,
            alignLeft = true,
            color = labels
        )
        drawHudLabel(canvas, "N", centerX, top + 8f * density, alignLeft = false, color = labels)
        drawHudLabel(
            canvas,
            "Vue AZ %.0f° · ALT %.0f° · %.0f°".format(lookAzimuth, lookAltitude, fieldOfViewDegrees * 2),
            left + 8f * density,
            bottom - 8f * density,
            alignLeft = true,
            color = labels
        )
        drawHudLabel(
            canvas,
            if (telemetryFresh) "ESP32 · DIRECT" else "Horizon",
            right - 8f * density,
            bottom - 8f * density,
            alignLeft = false,
            color = if (telemetryFresh) Color.rgb(129, 225, 171) else labels
        )
        canvas.restoreToCount(clipSave)

        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 1f * density
        paint.color = if (cameraOverlay) Color.argb(190, Color.red(grid), Color.green(grid), Color.blue(grid)) else grid
        canvas.drawRoundRect(scene, 14f * density, 14f * density, paint)
        paint.style = Paint.Style.FILL
    }

    private fun drawGridLine(
        canvas: Canvas,
        projection: FieldProjection,
        fixedCoordinate: Double,
        maximum: Double,
        step: Double,
        altitudeLine: Boolean
    ) {
        val path = Path()
        var drawing = false
        var value = 0.0
        while (value <= maximum) {
            val azimuth = if (altitudeLine) value else fixedCoordinate
            val altitude = if (altitudeLine) fixedCoordinate else value
            val point = projectToPhoneField(projection, azimuth, altitude)
            if (point == null) {
                drawing = false
            } else {
                if (drawing) path.lineTo(point.first, point.second) else path.moveTo(point.first, point.second)
                drawing = true
            }
            value += step
        }
        canvas.drawPath(path, paint)
    }

    private fun drawHudLabel(
        canvas: Canvas,
        label: String,
        x: Float,
        baseline: Float,
        alignLeft: Boolean,
        color: Int
    ) {
        textPaint.textAlign = if (alignLeft) Paint.Align.LEFT else Paint.Align.RIGHT
        textPaint.textSize = 9f * density
        textPaint.typeface = Typeface.DEFAULT
        val measured = textPaint.measureText(label)
        val horizontalPadding = 6f * density
        val verticalPadding = 4f * density
        val rectLeft = if (alignLeft) x - horizontalPadding else x - measured - horizontalPadding
        val rectRight = if (alignLeft) x + measured + horizontalPadding else x + horizontalPadding
        val rect = RectF(
            rectLeft,
            baseline - textPaint.textSize - verticalPadding,
            rectRight,
            baseline + verticalPadding
        )
        paint.style = Paint.Style.FILL
        paint.color = Color.argb(190, 5, 8, 14)
        canvas.drawRoundRect(rect, 5f * density, 5f * density, paint)
        textPaint.color = color
        canvas.drawText(label, x, baseline, textPaint)
        textPaint.textAlign = Paint.Align.CENTER
    }

    private fun objectColor(type: String): Int {
        val value = type.lowercase()
        return when {
            value.contains("plan") -> if (nightMode) Color.rgb(255, 170, 180) else Color.rgb(110, 199, 255)
            value.contains("gal") -> if (nightMode) Color.rgb(255, 170, 180) else Color.rgb(214, 171, 255)
            value.contains("neb") -> if (nightMode) Color.rgb(255, 170, 180) else Color.rgb(255, 145, 122)
            value.contains("amas") || value.contains("glob") ->
                if (nightMode) Color.rgb(255, 170, 180) else Color.rgb(255, 207, 107)
            else -> if (nightMode) Color.rgb(255, 180, 186) else Color.rgb(228, 238, 255)
        }
    }

    private fun matchesObjectFilter(item: SkyObject): Boolean {
        val type = item.type.lowercase()
        return when (objectTypeFilter) {
            "Galaxies" -> type.contains("gal")
            "Nébuleuses" -> type.contains("neb")
            "Amas d'étoiles" -> type.contains("amas") || type.contains("glob")
            "Étoiles" -> !type.contains("plan") && !type.contains("gal") &&
                !type.contains("neb") && !type.contains("amas") && !type.contains("glob")
            else -> true
        }
    }

    private fun projectToPhoneField(
        projection: FieldProjection,
        objectAzimuth: Double,
        objectAltitude: Double
    ): Pair<Float, Float>? {
        val azimuth = Math.toRadians(objectAzimuth)
        val altitude = Math.toRadians(objectAltitude)
        val cosAltitude = cos(altitude)
        val x = sin(azimuth) * cosAltitude
        val y = sin(altitude)
        val z = cos(azimuth) * cosAltitude
        val depth = x * projection.forwardX + y * projection.forwardY + z * projection.forwardZ
        if (depth <= 0.01) return null
        val screenX = (x * projection.rightX + z * projection.rightZ) /
            (depth * projection.tanHalfFov * projection.aspect)
        val screenY = (x * projection.upX + y * projection.upY + z * projection.upZ) /
            (depth * projection.tanHalfFov)
        return (projection.centerX + projection.halfWidth * screenX.toFloat()) to
            (projection.centerY - projection.halfHeight * screenY.toFloat())
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
        private const val DEFAULT_FIELD_OF_VIEW_DEGREES = 34.0
        private const val MIN_FIELD_OF_VIEW_DEGREES = 12.0
        private const val MAX_FIELD_OF_VIEW_DEGREES = 60.0
        private const val MAX_VISIBLE_OBJECTS = 40
    }
}
