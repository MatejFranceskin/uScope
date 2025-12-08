#include "CameraController.h"
#include "../services/CameraService.h"
#include "../services/PTPCameraService.h"
#include <QTimer>
#include <QSettings>

CameraController::CameraController(QObject* parent)
    : QObject(parent)
    , _service(nullptr)
    , _ptpService(nullptr)
    , _currentCameraType(CameraType::UVC)
    , _monitorTimer(nullptr)
    , _exposure(100.0)  // Default 100ms
    , _brightness(128)  // Default brightness
    , _contrast(32)     // Default contrast
    , _saturation(64)   // Default saturation
    , _flipHorizontal(false)
    , _flipVertical(false)
    , _whiteBalance(4600)  // Default 4600K
    , _autoExposure(true)   // Auto exposure on by default
    , _autoWhiteBalance(true)  // Auto white balance on by default
{
    _service = new CameraService(this);
    _ptpService = new PTPCameraService(this);
    
    // Forward signals from V4L2 service
    connect(_service, &CameraService::frameReady, 
            this, &CameraController::onFrameReady);
    connect(_service, &CameraService::cameraConnected, 
            this, &CameraController::onCameraConnected);
    connect(_service, &CameraService::cameraDisconnected, 
            this, &CameraController::onCameraDisconnected);
    connect(_service, &CameraService::error, 
            this, &CameraController::onServiceError);
    
    // Forward signals from PTP service
    connect(_ptpService, &PTPCameraService::frameReady,
            this, [this](const QImage& frame) {
                // Convert QImage to QVideoFrame for consistency
                QVideoFrameFormat format(frame.size(), QVideoFrameFormat::Format_RGBX8888);
                QVideoFrame videoFrame(format);
                if (videoFrame.map(QVideoFrame::WriteOnly)) {
                    QImage rgbxImage = frame.convertToFormat(QImage::Format_RGBX8888);
                    memcpy(videoFrame.bits(0), rgbxImage.constBits(), rgbxImage.sizeInBytes());
                    videoFrame.unmap();
                }
                emit frameReady(videoFrame);
            });
    connect(_ptpService, &PTPCameraService::error,
            this, &CameraController::onServiceError);
    
    // Create timer for camera monitoring (not started yet)
    _monitorTimer = new QTimer(this);
    _monitorTimer->setInterval(2000);  // Check every 2 seconds
    connect(_monitorTimer, &QTimer::timeout, this, &CameraController::checkForLastCamera);
}

CameraController::~CameraController()
{
    cleanupServices();
}

void CameraController::cleanupServices()
{
    if (_service) {
        _service->stopCamera();
    }
    if (_ptpService) {
        _ptpService->disconnect();
    }
}

CameraType CameraController::detectCameraType(const QString& cameraId) const
{
    if (cameraId.startsWith("ptp://")) {
        return CameraType::PTP;
    }
    return CameraType::UVC;
}

QList<CameraProfile> CameraController::availableCameras()
{
    QList<CameraProfile> cameras;
    
    // Get V4L2 cameras
    cameras.append(_service->enumerateCameras());
    
    // Get PTP cameras
    QList<PTPCameraInfo> ptpCameras = _ptpService->detectCameras();
    for (const PTPCameraInfo& info : ptpCameras) {
        CameraProfile profile(info.id, QString("%1 %2 (PTP)").arg(info.manufacturer, info.model));
        cameras.append(profile);
    }
    
    return cameras;
}

QList<QSize> CameraController::availableResolutions(const QString& cameraId)
{
    if (detectCameraType(cameraId) == CameraType::PTP) {
        // PTP cameras don't have resolution selection - return empty list
        return QList<QSize>();
    }
    return _service->availableResolutions(cameraId);
}

QList<double> CameraController::availableFrameRates(const QString& cameraId, const QSize& resolution)
{
    if (detectCameraType(cameraId) == CameraType::PTP) {
        // PTP cameras have fixed frame rate - return empty list
        return QList<double>();
    }
    return _service->availableFrameRates(cameraId, resolution);
}

QSize CameraController::currentResolution() const
{
    if (_currentCameraType == CameraType::PTP) {
        return QSize(); // PTP cameras handle resolution internally
    }
    return _service->currentResolution();
}

double CameraController::currentFrameRate() const
{
    if (_currentCameraType == CameraType::PTP) {
        return 0.0; // PTP cameras handle frame rate internally
    }
    return _service->currentFrameRate();
}

bool CameraController::isActive() const
{
    if (_currentCameraType == CameraType::PTP) {
        return _ptpService->isConnected();
    }
    return _service->isActive();
}

QString CameraController::currentCameraId() const
{
    return _service->currentCameraId();
}

void CameraController::startCamera(const QString& cameraId)
{
    qDebug() << "CameraController::startCamera (no resolution) - cameraId:" << cameraId;
    _currentCameraType = detectCameraType(cameraId);
    
    if (_currentCameraType == CameraType::PTP) {
        // Stop V4L2 camera if active
        _service->stopCamera();
        
        // Extract PTP camera info from available cameras
        QList<PTPCameraInfo> ptpCameras = _ptpService->detectCameras();
        for (const PTPCameraInfo& info : ptpCameras) {
            if (info.id == cameraId) {
                _ptpService->connect(info);
                break;
            }
        }
    } else {
        // Stop PTP camera if active
        _ptpService->disconnect();
        
        // Start V4L2 camera
        _service->startCamera(cameraId);
    }
}

void CameraController::startCamera(const QString& cameraId, const QSize& resolution, double frameRate)
{
    qDebug() << "CameraController::startCamera - cameraId:" << cameraId 
             << "resolution:" << resolution << "@" << frameRate << "fps";
    
    _currentCameraType = detectCameraType(cameraId);
    
    if (_currentCameraType == CameraType::PTP) {
        // Stop V4L2 camera if active
        _service->stopCamera();
        
        // Extract PTP camera info from available cameras
        QList<PTPCameraInfo> ptpCameras = _ptpService->detectCameras();
        for (const PTPCameraInfo& info : ptpCameras) {
            if (info.id == cameraId) {
                _ptpService->connect(info);
                break;
            }
        }
    } else {
        // Stop PTP camera if active
        _ptpService->disconnect();
        
        // Start V4L2 camera
        _service->startCamera(cameraId, resolution, frameRate);
    }
}

void CameraController::stopCamera()
{
    if (_currentCameraType == CameraType::PTP) {
        _ptpService->disconnect();
    } else {
        _service->stopCamera();
    }
}

void CameraController::saveCurrentCamera()
{
    QSettings settings("uScope", "uScope");
    settings.setValue("camera/lastCameraName", _lastCameraName);
    
    qDebug() << "CameraController::saveCurrentCamera - cameraName:" << _lastCameraName;
}

void CameraController::saveCameraSelection(const QString& cameraId, const QSize& resolution)
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
             << "name:" << cameraName << "resolution:" << resolution;
    _lastCameraName = cameraName;
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
    
    // Try to start the last camera immediately by searching for the name
    QList<CameraProfile> cameras = availableCameras();
    qDebug() << "CameraController::restoreLastCamera - found" << cameras.size() << "available cameras";
    
    for (const CameraProfile& camera : cameras) {
        if (camera.name() == _lastCameraName) {
            qDebug() << "CameraController::restoreLastCamera - found saved camera:" << camera.name() << "id:" << camera.id();
            // Camera found, try to restore with same resolution
            QSize savedResolution = getSavedResolution(camera.id());
            if (savedResolution.isValid()) {
                QList<QSize> resolutions = availableResolutions(camera.id());
                qDebug() << "CameraController::restoreLastCamera - checking" << resolutions.size() << "resolutions for match";
                for (const QSize& res : resolutions) {
                    if (res == savedResolution) {
                        qDebug() << "CameraController::restoreLastCamera - found matching resolution, starting camera";
                        startCamera(camera.id(), res);
                        return;
                    }
                }
                qDebug() << "CameraController::restoreLastCamera - exact resolution not found, starting with default";
            }
            // Resolution not found, start with default
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

QMap<QString, QVariant> CameraController::getPTPCapabilities() const
{
    if (_currentCameraType == CameraType::PTP && _ptpService) {
        return _ptpService->getCapabilities();
    }
    return QMap<QString, QVariant>();
}

void CameraController::setPTPSetting(const QString& name, const QString& value)
{
    if (_currentCameraType == CameraType::PTP && _ptpService) {
        _ptpService->setSetting(name, value);
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
            
            QSize savedResolution = getSavedResolution(camera.id());
            if (savedResolution.isValid()) {
                QList<QSize> resolutions = availableResolutions(camera.id());
                for (const QSize& res : resolutions) {
                    if (res == savedResolution) {
                        startCamera(camera.id(), res);
                        return;
                    }
                }
            }
            // Resolution not found, start with default
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
    // Restore saved camera controls for this camera
    restoreCameraControls(cameraId);
    
    // Apply the restored settings to the camera
    _service->setAutoExposure(_autoExposure);
    _service->setAutoWhiteBalance(_autoWhiteBalance);
    _service->setBrightness(_brightness);
    _service->setContrast(_contrast);
    _service->setSaturation(_saturation);
    _service->setFlipHorizontal(_flipHorizontal);
    _service->setFlipVertical(_flipVertical);
    
    // Only apply manual settings if auto modes are disabled
    if (!_autoExposure) {
        _service->setExposure(_exposure);
    }
    if (!_autoWhiteBalance) {
        _service->setWhiteBalance(_whiteBalance);
    }
    
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

// Camera controls (US3) - OpenCV-based
void CameraController::setExposure(int value)
{
    _exposure = value;
    _autoExposure = false;  // Disable auto when manually adjusting
    _service->setExposure(value);
    saveCameraControls(currentCameraId());
}

void CameraController::setGain(int value)
{
    _service->setGain(value);
    // Note: Gain not currently persisted
}

void CameraController::setWhiteBalance(int value)
{
    _whiteBalance = value;
    _autoWhiteBalance = false;  // Disable auto when manually adjusting
    _service->setWhiteBalance(value);
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

void CameraController::setAutoExposure(bool enabled)
{
    _autoExposure = enabled;
    _service->setAutoExposure(enabled);
    saveCameraControls(currentCameraId());
}

void CameraController::setAutoWhiteBalance(bool enabled)
{
    _autoWhiteBalance = enabled;
    _service->setAutoWhiteBalance(enabled);
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
    settings.setValue(prefix + "autoExposure", _autoExposure);
    settings.setValue(prefix + "autoWhiteBalance", _autoWhiteBalance);
    
    // Save resolution for this camera
    if (currentResolution().isValid()) {
        settings.setValue(prefix + "resolution", currentResolution());
    }
    
    // Removed excessive debug output
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
    _brightness = settings.value(prefix + "brightness", 128).toInt();
    _contrast = settings.value(prefix + "contrast", 32).toInt();
    _saturation = settings.value(prefix + "saturation", 64).toInt();
    _flipHorizontal = settings.value(prefix + "flipHorizontal", false).toBool();
    _flipVertical = settings.value(prefix + "flipVertical", false).toBool();
    _whiteBalance = settings.value(prefix + "whiteBalance", 4600).toInt();  // Default 4600K
    _autoExposure = settings.value(prefix + "autoExposure", true).toBool();
    _autoWhiteBalance = settings.value(prefix + "autoWhiteBalance", true).toBool();
    
    qDebug() << "CameraController::restoreCameraControls - restored for" << cameraId
             << "exposure:" << _exposure << "brightness:" << _brightness;
}

QSize CameraController::getSavedResolution(const QString& cameraId) const
{
    if (cameraId.isEmpty()) {
        return QSize();  // Invalid
    }
    
    QSettings settings("uScope", "uScope");
    QString prefix = QString("camera/%1/controls/").arg(cameraId);
    
    return settings.value(prefix + "resolution", QSize()).toSize();
}

int CameraController::getCurrentExposure() const
{
    return _service->getCurrentExposure();
}

int CameraController::getCurrentWhiteBalance() const
{
    return _service->getCurrentWhiteBalance();
}
