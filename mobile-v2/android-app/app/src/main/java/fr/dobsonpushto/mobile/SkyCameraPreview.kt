package fr.dobsonpushto.mobile

import android.content.Context
import android.graphics.SurfaceTexture
import android.hardware.camera2.CameraCaptureSession
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraDevice
import android.hardware.camera2.CameraManager
import android.hardware.camera2.CaptureRequest
import android.os.Handler
import android.os.HandlerThread
import android.view.Surface
import android.view.TextureView
import android.widget.FrameLayout

class SkyCameraPreview(
    context: Context,
    private val onError: (String) -> Unit
) : FrameLayout(context) {
    private val textureView = TextureView(context)
    private var cameraThread: HandlerThread? = null
    private var cameraHandler: Handler? = null
    private var cameraDevice: CameraDevice? = null
    private var captureSession: CameraCaptureSession? = null
    private var previewSurface: Surface? = null
    private var requested = false

    init {
        addView(textureView, LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT))
        textureView.surfaceTextureListener = object : TextureView.SurfaceTextureListener {
            override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
                if (requested) openCamera()
            }

            override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) = Unit

            override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
                closeCamera()
                return true
            }

            override fun onSurfaceTextureUpdated(surface: SurfaceTexture) = Unit
        }
    }

    fun start() {
        requested = true
        if (cameraThread == null) {
            cameraThread = HandlerThread("dobson-sky-camera").also { it.start() }
            cameraHandler = Handler(cameraThread!!.looper)
        }
        if (textureView.isAvailable) openCamera()
    }

    fun stop() {
        requested = false
        closeCamera()
        cameraThread?.quitSafely()
        cameraThread = null
        cameraHandler = null
    }

    override fun onDetachedFromWindow() {
        stop()
        super.onDetachedFromWindow()
    }

    private fun openCamera() {
        if (!requested || cameraDevice != null) return
        try {
            val manager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
            val cameraId = manager.cameraIdList.firstOrNull { id ->
                manager.getCameraCharacteristics(id).get(CameraCharacteristics.LENS_FACING) ==
                    CameraCharacteristics.LENS_FACING_BACK
            } ?: manager.cameraIdList.firstOrNull()
                ?: throw IllegalStateException("Aucune caméra disponible.")
            manager.openCamera(cameraId, object : CameraDevice.StateCallback() {
                override fun onOpened(camera: CameraDevice) {
                    if (!requested) {
                        camera.close()
                        return
                    }
                    cameraDevice = camera
                    startPreview(cameraId)
                }

                override fun onDisconnected(camera: CameraDevice) {
                    camera.close()
                    if (cameraDevice === camera) cameraDevice = null
                    onError("La caméra a été déconnectée.")
                }

                override fun onError(camera: CameraDevice, error: Int) {
                    camera.close()
                    if (cameraDevice === camera) cameraDevice = null
                    onError("Erreur caméra Android ($error).")
                }
            }, cameraHandler)
        } catch (error: Exception) {
            onError("Impossible d'ouvrir la caméra : ${error.message ?: "erreur Android"}.")
        }
    }

    private fun startPreview(cameraId: String) {
        val camera = cameraDevice ?: return
        val texture = textureView.surfaceTexture ?: return
        try {
            val manager = context.getSystemService(Context.CAMERA_SERVICE) as CameraManager
            val characteristics = manager.getCameraCharacteristics(cameraId)
            val outputSizes = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                ?.getOutputSizes(SurfaceTexture::class.java)
                .orEmpty()
            val previewSize = outputSizes
                .filter { it.width <= 1920 && it.height <= 1080 }
                .maxByOrNull { it.width.toLong() * it.height }
                ?: outputSizes.firstOrNull()
                ?: throw IllegalStateException("La caméra ne propose aucun format d'aperçu.")
            texture.setDefaultBufferSize(previewSize.width, previewSize.height)
            previewSurface = Surface(texture)
            val surface = previewSurface ?: return
            val request = camera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW).apply {
                addTarget(surface)
                set(CaptureRequest.CONTROL_MODE, CaptureRequest.CONTROL_MODE_AUTO)
            }.build()
            camera.createCaptureSession(listOf(surface), object : CameraCaptureSession.StateCallback() {
                override fun onConfigured(session: CameraCaptureSession) {
                    if (!requested || cameraDevice !== camera) {
                        session.close()
                        return
                    }
                    captureSession = session
                    try {
                        session.setRepeatingRequest(request, null, cameraHandler)
                    } catch (error: Exception) {
                        onError("Impossible de démarrer l'aperçu caméra : ${error.message ?: "erreur Android"}.")
                    }
                }

                override fun onConfigureFailed(session: CameraCaptureSession) {
                    session.close()
                    onError("La caméra ne peut pas démarrer l'aperçu.")
                }
            }, cameraHandler)
        } catch (error: Exception) {
            onError("Impossible de préparer l'aperçu caméra : ${error.message ?: "erreur Android"}.")
        }
    }

    private fun closeCamera() {
        captureSession?.close()
        captureSession = null
        cameraDevice?.close()
        cameraDevice = null
        previewSurface?.release()
        previewSurface = null
    }
}
