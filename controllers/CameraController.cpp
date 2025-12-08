#include "CameraController.h"
#include "../services/CameraService.h"
#include <QTimer>
#include <QSettings>

CameraController::CameraController(QObject* parent)
    : QObject(parent)
    , _monitorTimer(nullptr)
    , _exposure(100.0)  // Default 100ms
    , _brightness(0)
    , _contrast(0)
    , _saturation(0)
    , _flipHorizontal(false)
    , _flipVertical(false)
    , _whiteBalance(static_cast<int>(QCamera::WhiteBalanceAuto))
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
    
    // Create timer for camera monitoring (not started yet)
    _monitorTimer = new QTimer(this);
    _monitorTimer->setInterval(2000);  // Check every 2 seconds
    connect(_monitorTimer, &QTimer::timeout, this, &CameraController::checkForLastCamera);
}

CameraController::~CameraController()
{
    // CameraService deleted by Qt parent-child ownership
}

QList<CameraProfile> CameraController::availableCameras()
{
    return _service->enumerateCameras();
}

QList<QCameraFormat> CameraController::availableFormats(const QString& cameraId)
{
    return _service->availableFormats(cameraId);
}

QCameraFormat CameraController::currentFormat() const
{
    return _service->currentFormat();
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
    qDebug() << "CameraController::startCamera (no format) - cameraId:" << cameraId;
    qDebug() << "Stack trace: called from somewhere - this should only be called for fallback!";
    _lastResolution = QSize();
    _lastFrameRate = 0.0;
    _service->startCamera(cameraId);
}

void CameraController::startCamera(const QString& cameraId, const QCameraFormat& format)
{
    qDebug() << "CameraController::startCamera (with format) - cameraId:" << cameraId 
             << "resolution:" << format.resolution() << "fps:" << format.maxFrameRate();
    _lastResolution = format.resolution();
    _lastFrameRate = format.maxFrameRate();
    _service->startCamera(cameraId, format);
}

void CameraController::stopCamera()
{
    _service->stopCamera();
}

void CameraController::saveCurrentCamera()
{
    QSettings settings("uScope", "uScope");
    settings.setValue("camera/lastCameraName", _lastCameraName);
    
    qDebug() << "CameraController::saveCurrentCamera - cameraName:" << _lastCameraName;
    
    if (_lastResolution.isValid() && _lastFrameRate > 0) {
        settings.setValue("camera/lastResolutionWidth", _lastResolution.width());
        settings.setValue("camera/lastResolutionHeight", _lastResolution.height());
        settings.setValue("camera/lastFrameRate", _lastFrameRate);
        qDebug() << "CameraController::saveCurrentCamera - resolution:" << _lastResolution << "fps:" << _lastFrameRate;
    } else {
        qDebug() << "CameraController::saveCurrentCamera - no valid format to save";
    }
}

void CameraController::saveCameraSelection(const QString& cameraId, const QCameraFormat& format)
{
    // Find camera name from available cameras
    QString cameraName;
    QList<CameraProfile> cameras = availableCameras();
    for (const CameraProfile& camera : cameras) {
        if (camera.id() == cameraId) {
            cameraName = camera.name();
            break;
        }
    }
    
    qDebug() << "CameraController::saveCameraSelection - cameraId:" << cameraId 
             << "name:" << cameraName << "resolution:" << format.resolution() << "fps:" << format.maxFrameRate();
    _lastCameraName = cameraName;
    _lastResolution = format.resolution();
    _lastFrameRate = format.maxFrameRate();
    saveCurrentCamera();
}

void CameraController::restoreLastCamera()
{
    QSettings settings("uScope", "uScope");
    _lastCameraName = settings.value("camera/lastCameraName").toString();
    
    qDebug() << "CameraController::restoreLastCamera - read cameraName:" << _lastCameraName;
    
    if (_lastCameraName.isEmpty()) {
        qDebug() << "CameraController::restoreLastCamera - no saved camera, returning";
        return;  // No saved camera
    }
    
    _lastResolution = QSize(
        settings.value("camera/lastResolutionWidth", 0).toInt(),
        settings.value("camera/lastResolutionHeight", 0).toInt()
    );
    _lastFrameRate = settings.value("camera/lastFrameRate", 0.0).toDouble();
    
    qDebug() << "CameraController::restoreLastCamera - resolution:" << _lastResolution << "fps:" << _lastFrameRate;
    
    // Try to start the last camera immediately by searching for the name
    QList<CameraProfile> cameras = availableCameras();
    qDebug() << "CameraController::restoreLastCamera - found" << cameras.size() << "available cameras";
    
    for (const CameraProfile& camera : cameras) {
        if (camera.name() == _lastCameraName) {
            qDebug() << "CameraController::restoreLastCamera - found saved camera:" << camera.name() << "id:" << camera.id();
            // Camera found, try to restore with same format
            if (_lastResolution.isValid() && _lastFrameRate > 0) {
                QList<QCameraFormat> formats = availableFormats(camera.id());
                qDebug() << "CameraController::restoreLastCamera - checking" << formats.size() << "formats for exact match";
                for (const QCameraFormat& format : formats) {
                    if (format.resolution() == _lastResolution && 
                        qAbs(format.maxFrameRate() - _lastFrameRate) < 0.1) {
                        qDebug() << "CameraController::restoreLastCamera - found matching format, starting camera";
                        startCamera(camera.id(), format);
                        return;
                    }
                }
                qDebug() << "CameraController::restoreLastCamera - exact format not found, starting with default";
            }
            // Format not found, start with default
            startCamera(camera.id());
            return;
        }
    }
    
    qDebug() << "CameraController::restoreLastCamera - camera not found, starting monitoring";
    // Camera not found, start monitoring for reconnection
    startCameraMonitoring();
}

void CameraController::startCameraMonitoring()
{
    if (_monitorTimer && !_lastCameraName.isEmpty()) {
        _monitorTimer->start();
    }
}

void CameraController::stopCameraMonitoring()
{
    if (_monitorTimer) {
        _monitorTimer->stop();
    }
}

void CameraController::checkForLastCamera()
{
    if (_lastCameraName.isEmpty() || isActive()) {
        stopCameraMonitoring();
        return;
    }
    
    // Check if last camera is now available by searching for the name
    QList<CameraProfile> cameras = availableCameras();
    for (const CameraProfile& camera : cameras) {
        if (camera.name() == _lastCameraName) {
            // Found the camera, try to restore it
            stopCameraMonitoring();
            
            if (_lastResolution.isValid() && _lastFrameRate > 0) {
                QList<QCameraFormat> formats = availableFormats(camera.id());
                for (const QCameraFormat& format : formats) {
                    if (format.resolution() == _lastResolution && 
                        qAbs(format.maxFrameRate() - _lastFrameRate) < 0.1) {
                        startCamera(camera.id(), format);
                        return;
                    }
                }
            }
            // Format not found, start with default
            startCamera(camera.id());
            return;
        }
    }
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
    // Don't auto-start if a camera is already active (e.g., from restoreLastCamera)
    if (isActive()) {
        qDebug() << "CameraController::autoStartCamera - camera already active, skipping";
        return;
    }
    
    QList<CameraProfile> cameras = availableCameras();
    
    if (cameras.isEmpty()) {
        emit error("No cameras detected");
        return;
    }
    
    // Start first available camera
    qDebug() << "CameraController::autoStartCamera - starting first available camera";
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
    
    // Note: We can't compare by cameraId since we now use camera name.
    // Just start monitoring when any camera disconnects and we have a saved camera.
    // The monitoring will check if it's the right camera when it tries to reconnect.
    if (!_lastCameraName.isEmpty()) {
        startCameraMonitoring();
    }
}

void CameraController::onServiceError(const QString& message)
{
    emit error(message);
}

// Camera controls (US3)
void CameraController::setExposure(qreal value)
{
    _exposure = value;
    _service->setExposure(value);
    saveCameraControls(currentCameraId());
}

void CameraController::setWhiteBalance(int mode)
{
    _whiteBalance = mode;
    _service->setWhiteBalance(static_cast<QCamera::WhiteBalanceMode>(mode));
    saveCameraControls(currentCameraId());
}

void CameraController::setBrightness(int value)
{
    _brightness = value;
    _service->setBrightness(value);
    saveCameraControls(currentCameraId());
}

void CameraController::setContrast(int value)
{
    _contrast = value;
    _service->setContrast(value);
    saveCameraControls(currentCameraId());
}

void CameraController::setSaturation(int value)
{
    _saturation = value;
    _service->setSaturation(value);
    saveCameraControls(currentCameraId());
}

void CameraController::setFlipHorizontal(bool enabled)
{
    _flipHorizontal = enabled;
    _service->setFlipHorizontal(enabled);
    saveCameraControls(currentCameraId());
}

void CameraController::setFlipVertical(bool enabled)
{
    _flipVertical = enabled;
    _service->setFlipVertical(enabled);
    saveCameraControls(currentCameraId());
}

void CameraController::saveCameraControls(const QString& cameraId)
{
    if (cameraId.isEmpty()) {
        return;  // No camera selected
    }
    
    QSettings settings("uScope", "uScope");
    QString prefix = QString("camera/%1/controls/").arg(cameraId);
    
    settings.setValue(prefix + "exposure", _exposure);
    settings.setValue(prefix + "brightness", _brightness);
    settings.setValue(prefix + "contrast", _contrast);
    settings.setValue(prefix + "saturation", _saturation);
    settings.setValue(prefix + "flipHorizontal", _flipHorizontal);
    settings.setValue(prefix + "flipVertical", _flipVertical);
    settings.setValue(prefix + "whiteBalance", _whiteBalance);
    
    qDebug() << "CameraController::saveCameraControls - saved for" << cameraId;
}

void CameraController::restoreCameraControls(const QString& cameraId)
{
    if (cameraId.isEmpty()) {
        return;  // No camera selected
    }
    
    QSettings settings("uScope", "uScope");
    QString prefix = QString("camera/%1/controls/").arg(cameraId);
    
    // Restore with defaults if not found
    _exposure = settings.value(prefix + "exposure", 100.0).toReal();
    _brightness = settings.value(prefix + "brightness", 0).toInt();
    _contrast = settings.value(prefix + "contrast", 0).toInt();
    _saturation = settings.value(prefix + "saturation", 0).toInt();
    _flipHorizontal = settings.value(prefix + "flipHorizontal", false).toBool();
    _flipVertical = settings.value(prefix + "flipVertical", false).toBool();
    _whiteBalance = settings.value(prefix + "whiteBalance", static_cast<int>(QCamera::WhiteBalanceAuto)).toInt();
    
    qDebug() << "CameraController::restoreCameraControls - restored for" << cameraId
             << "exposure:" << _exposure << "brightness:" << _brightness;
}

