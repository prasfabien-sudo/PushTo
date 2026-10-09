package fr.dobsonpushto.mobile

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.view.View

class GuidanceView(context: Context) : View(context) {
    var azimuthError = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var altitudeError = 0.0
        set(value) {
            field = value
            invalidate()
        }
    var nightMode = false
        set(value) {
            field = value
            invalidate()
        }

    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val density = resources.displayMetrics.density
        val warning = Color.rgb(255, 82, 82)
        val onTarget = if (nightMode) Color.rgb(255, 180, 180) else Color.rgb(76, 175, 80)
        val secondary = if (nightMode) Color.rgb(180, 70, 70) else Color.rgb(84, 110, 122)
        val accent = if (nightMode) Color.rgb(255, 100, 100) else Color.rgb(255, 183, 77)
        val gaugeWidth = width * 0.58f
        val centerX = gaugeWidth * 0.5f
        val centerY = height * 0.5f
        val compassRadius = (height * 0.34f).coerceAtLeast(22f * density)

        paint.style = Paint.Style.STROKE
        paint.strokeWidth = 1.5f * density
        paint.color = accent
        canvas.drawCircle(centerX, centerY, compassRadius, paint)
        paint.style = Paint.Style.FILL
        paint.textAlign = Paint.Align.CENTER
        paint.textSize = 11f * density
        paint.color = accent
        canvas.drawText("N", centerX, centerY - compassRadius + 13f * density, paint)

        val azOk = kotlin.math.abs(azimuthError) < 0.5
        val azAngle = Math.toRadians(azimuthError)
        val needleRadius = compassRadius - 10f * density
        val needleX = centerX + needleRadius * kotlin.math.sin(azAngle).toFloat()
        val needleY = centerY - needleRadius * kotlin.math.cos(azAngle).toFloat()
        paint.color = if (azOk) onTarget else warning
        paint.strokeWidth = 2.5f * density
        canvas.drawLine(centerX, centerY, needleX, needleY, paint)
        canvas.drawCircle(needleX, needleY, 4f * density, paint)
        paint.color = primaryText()
        canvas.drawText("AZ %.1f°".format(azimuthError), centerX, height - 6f * density, paint)

        val barX = width * 0.83f
        val barTop = height * 0.13f
        val barBottom = height * 0.87f
        paint.color = secondary
        paint.strokeWidth = 8f * density
        paint.strokeCap = Paint.Cap.ROUND
        canvas.drawLine(barX, barTop, barX, barBottom, paint)
        paint.strokeCap = Paint.Cap.BUTT
        paint.color = accent
        paint.strokeWidth = 2f * density
        canvas.drawLine(barX - 16f * density, centerY, barX + 16f * density, centerY, paint)
        val y = (centerY - altitudeError.coerceIn(-15.0, 15.0).toFloat() / 15f *
            (height * 0.34f)).coerceIn(barTop, barBottom)
        val altOk = kotlin.math.abs(altitudeError) < 0.5
        paint.color = if (altOk) onTarget else warning
        paint.strokeWidth = 4f * density
        canvas.drawLine(barX - 16f * density, y, barX + 16f * density, y, paint)
        paint.color = primaryText()
        paint.textSize = 10f * density
        canvas.drawText("ALT", barX, barTop - 4f * density, paint)
        canvas.drawText("%+.1f°".format(altitudeError), barX, height - 6f * density, paint)
    }

    private fun primaryText(): Int =
        if (nightMode) Color.rgb(255, 190, 190) else Color.WHITE
}
