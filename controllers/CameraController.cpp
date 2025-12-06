#include "CameraController.h"
#include "../services/CameraService.h"

CameraController::CameraController(QObject* parent)
    : QObject(parent)
{
    _service = new CameraService(this);
    
    // Forward signals from service
    connect(_service, &CameraService::frameReady, 
            this, &CameraController::onFrameReady);
    connect(_service, &CameraService::cameraConnected, 
            this, &CameraController::onCameraConnected);
    connect(_service, &CameraService::cameraDisconnected, 
            this, &CameraController::onCameraDisconnected);
    connect(_service, &CameraService::error, 
            this, &CameraController::onServiceError);
}

CameraController::~CameraController()
{
    // CameraService deleted by Qt parent-child ownership
}

QList<CameraProfile> CameraController::availableCameras()
{
    return _service->enumerateCameras();
}

bool CameraController::isActive() const
{
    return _service->isActive();
}

QString CameraController::currentCameraId() const
{
    return _service->currentCameraId();
}

void CameraController::startCamera(const QString& cameraId)
{
    _service->startCamera(cameraId);
}

void CameraController::stopCamera()
{
    _service->stopCamera();
}

void CameraController::captureImage()
{
    if (!_service->isActive()) {
        emit error("No active camera for capture");
        return;
    }
    
    QImage frame = _service->captureFrame();
    if (frame.isNull()) {
        emit error("Failed to capture frame");
        return;
    }
    
    CapturedImage image(frame, _service->currentCameraId());
    
    if (image.save()) {
        emit imageCaptured(image);
    } else {
        emit error("Failed to save captured image");
    }
}

void CameraController::autoStartCamera()
{
    QList<CameraProfile> cameras = availableCameras();
    
    if (cameras.isEmpty()) {
        emit error("No cameras detected");
        return;
    }
    
    // Start first available camera
    startCamera(cameras.first().id());
}

void CameraController::onFrameReady(const QVideoFrame& frame)
{
    emit frameReady(frame);
}

void CameraController::onCameraConnected(const QString& cameraId, const QString& name)
{
    emit cameraConnected(cameraId, name);
}

void CameraController::onCameraDisconnected(const QString& cameraId)
{
    emit cameraDisconnected(cameraId);
}

void CameraController::onServiceError(const QString& message)
{
    emit error(message);
}
