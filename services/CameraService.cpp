#include "CameraService.h"
#include <QMediaDevices>
#include <QCameraDevice>
#include <QCameraFormat>

CameraService::CameraService(QObject* parent)
    : QObject(parent)
    , _camera(nullptr)
    , _captureSession(nullptr)
    , _videoSink(nullptr)
{
    _captureSession = new QMediaCaptureSession(this);
    _videoSink = new QVideoSink(this);
    
    connect(_videoSink, &QVideoSink::videoFrameChanged, 
            this, &CameraService::onVideoFrameChanged);
}

CameraService::~CameraService()
{
    stopCamera();
}

QList<CameraProfile> CameraService::enumerateCameras()
{
    QList<CameraProfile> cameras;
    
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    for (const QCameraDevice& device : devices) {
        CameraProfile profile;
        profile.setId(device.id());
        profile.setName(device.description());
        
        // Get preferred resolution from default format
        if (!device.videoFormats().isEmpty()) {
            QCameraFormat format = device.videoFormats().first();
            profile.setResolution(format.resolution());
            profile.setFrameRate(static_cast<int>(format.maxFrameRate()));
        }
        
        cameras.append(profile);
    }
    
    return cameras;
}

QList<QCameraFormat> CameraService::availableFormats(const QString& cameraId)
{
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            return device.videoFormats();
        }
    }
    
    return QList<QCameraFormat>();
}

QCameraFormat CameraService::currentFormat() const
{
    if (_camera) {
        return _camera->cameraFormat();
    }
    return QCameraFormat();
}

bool CameraService::startCamera(const QString& cameraId)
{
    stopCamera();
    
    // Find camera device by ID
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    QCameraDevice selectedDevice;
    
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            selectedDevice = device;
            break;
        }
    }
    
    if (selectedDevice.isNull()) {
        emit error(QString("Camera not found: %1").arg(cameraId));
        return false;
    }
    
    // Create camera instance
    _camera = new QCamera(selectedDevice, this);
    
    // Connect error signal
    connect(_camera, &QCamera::errorOccurred, 
            this, &CameraService::onCameraErrorOccurred);
    
    // Configure capture session
    _captureSession->setCamera(_camera);
    _captureSession->setVideoSink(_videoSink);
    
    // Start camera
    _camera->start();
    
    if (_camera->isActive()) {
        _currentCameraId = cameraId;
        emit cameraConnected(cameraId, selectedDevice.description());
        return true;
    } else {
        delete _camera;
        _camera = nullptr;
        emit error("Failed to start camera");
        return false;
    }
}

bool CameraService::startCamera(const QString& cameraId, const QCameraFormat& format)
{
    stopCamera();
    
    qDebug() << "CameraService::startCamera - requested format:" << format.resolution() << "@" << format.maxFrameRate();
    
    // Find camera device by ID
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    QCameraDevice selectedDevice;
    
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            selectedDevice = device;
            break;
        }
    }
    
    if (selectedDevice.isNull()) {
        emit error(QString("Camera not found: %1").arg(cameraId));
        return false;
    }
    
    // Create camera instance
    _camera = new QCamera(selectedDevice, this);
    
    // Set the specific format
    _camera->setCameraFormat(format);
    qDebug() << "CameraService::startCamera - format set, camera format is now:" << _camera->cameraFormat().resolution() << "@" << _camera->cameraFormat().maxFrameRate();
    
    // Connect error signal
    connect(_camera, &QCamera::errorOccurred, 
            this, &CameraService::onCameraErrorOccurred);
    
    // Configure capture session
    _captureSession->setCamera(_camera);
    _captureSession->setVideoSink(_videoSink);
    
    // Start camera
    _camera->start();
    
    qDebug() << "CameraService::startCamera - camera started, active:" << _camera->isActive();
    qDebug() << "CameraService::startCamera - camera format after start:" << _camera->cameraFormat().resolution() << "@" << _camera->cameraFormat().maxFrameRate();
    
    if (_camera->isActive()) {
        _currentCameraId = cameraId;
        emit cameraConnected(cameraId, selectedDevice.description());
        return true;
    } else {
        delete _camera;
        _camera = nullptr;
        emit error("Failed to start camera");
        return false;
    }
}

void CameraService::stopCamera()
{
    if (_camera) {
        _camera->stop();
        _captureSession->setCamera(nullptr);
        delete _camera;
        _camera = nullptr;
        
        if (!_currentCameraId.isEmpty()) {
            emit cameraDisconnected(_currentCameraId);
            _currentCameraId.clear();
        }
    }
}

bool CameraService::isActive() const
{
    return _camera && _camera->isActive();
}

QImage CameraService::captureFrame()
{
    if (_lastFrame.isValid()) {
        return _lastFrame.toImage();
    }
    return QImage();
}

void CameraService::onVideoFrameChanged(const QVideoFrame& frame)
{
    if (frame.isValid()) {
        _lastFrame = frame;
        
#ifdef ENABLE_LATENCY_MEASUREMENT
        // Measure latency for SC-004 verification (<200ms requirement)
        if (_controlChangeTimer.isValid() && !_lastControlChange.isEmpty()) {
            qint64 latencyMs = _controlChangeTimer.elapsed();
            qDebug() << "CameraService::onVideoFrameChanged - control change latency:"
                     << _lastControlChange << "took" << latencyMs << "ms"
                     << (latencyMs < 200 ? "✓ PASS" : "✗ FAIL");
            _lastControlChange.clear();
        }
#endif
        
        emit frameReady(frame);
    }
}

void CameraService::onCameraErrorOccurred(QCamera::Error error, const QString& errorString)
{
    Q_UNUSED(error);
    emit this->error(QString("Camera error: %1").arg(errorString));
}

// Camera controls (US3)
void CameraService::setExposure(qreal value)
{
    if (!_camera || !_camera->isActive()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("exposure=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    if (_camera->isExposureModeSupported(QCamera::ExposureManual)) {
        _camera->setExposureMode(QCamera::ExposureManual);
        _camera->setManualExposureTime(value);
    }
}

void CameraService::setWhiteBalance(QCamera::WhiteBalanceMode mode)
{
    if (!_camera || !_camera->isActive()) {
        return;
    }
    
    if (_camera->isWhiteBalanceModeSupported(mode)) {
        _camera->setWhiteBalanceMode(mode);
    }
}

void CameraService::setBrightness(int value)
{
    if (!_camera || !_camera->isActive()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("brightness=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // Clamp to -100..100 range
    qreal normalized = qBound(-1.0, value / 100.0, 1.0);
    _camera->setColorTemperature(6500 + (normalized * 2000));  // Basic brightness via color temp
}

void CameraService::setContrast(int value)
{
    if (!_camera || !_camera->isActive()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("contrast=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // Qt6 doesn't have direct contrast control
    // This would need QVideoSink shader processing
    Q_UNUSED(value);
}

void CameraService::setSaturation(int value)
{
    if (!_camera || !_camera->isActive()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("saturation=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // Qt6 doesn't have direct saturation control
    // This would need QVideoSink shader processing
    Q_UNUSED(value);
}

void CameraService::setFlipHorizontal(bool enabled)
{
    // TODO: Implement via QVideoSink transformation
    Q_UNUSED(enabled);
}

void CameraService::setFlipVertical(bool enabled)
{
    // TODO: Implement via QVideoSink transformation
    Q_UNUSED(enabled);
}

void CameraService::autoWhiteBalance()
{
    if (!_camera || !_lastFrame.isValid()) {
        qDebug() << "CameraService::autoWhiteBalance - no valid frame available";
        return;
    }
    
    // Convert frame to image for processing
    QImage frame = _lastFrame.toImage();
    if (frame.isNull()) {
        qDebug() << "CameraService::autoWhiteBalance - failed to convert frame to image";
        return;
    }
    
    // Sample center 10% region of the frame
    int centerX = frame.width() / 2;
    int centerY = frame.height() / 2;
    int sampleWidth = frame.width() / 10;
    int sampleHeight = frame.height() / 10;
    
    int startX = centerX - sampleWidth / 2;
    int startY = centerY - sampleHeight / 2;
    int endX = centerX + sampleWidth / 2;
    int endY = centerY + sampleHeight / 2;
    
    // Calculate average RGB values in the sampled region
    qint64 sumR = 0, sumG = 0, sumB = 0;
    int pixelCount = 0;
    
    for (int y = startY; y < endY && y < frame.height(); ++y) {
        for (int x = startX; x < endX && x < frame.width(); ++x) {
            QRgb pixel = frame.pixel(x, y);
            sumR += qRed(pixel);
            sumG += qGreen(pixel);
            sumB += qBlue(pixel);
            ++pixelCount;
        }
    }
    
    if (pixelCount == 0) {
        qDebug() << "CameraService::autoWhiteBalance - no pixels sampled";
        return;
    }
    
    // Calculate averages
    double avgR = static_cast<double>(sumR) / pixelCount;
    double avgG = static_cast<double>(sumG) / pixelCount;
    double avgB = static_cast<double>(sumB) / pixelCount;
    
    qDebug() << "CameraService::autoWhiteBalance - sampled" << pixelCount << "pixels, avg RGB:" 
             << avgR << avgG << avgB;
    
    // Calculate color temperature adjustment
    // Higher red means warmer (lower color temp needed)
    // Higher blue means cooler (higher color temp needed)
    double colorRatio = avgB / (avgR + 1.0);  // +1 to avoid division by zero
    
    // Map ratio to color temperature (typical range 2500K-9000K)
    // Neutral white is around 6500K
    int colorTemp = 6500;
    
    if (colorRatio > 1.1) {
        // Too much blue - increase temperature (warmer)
        colorTemp = 6500 + static_cast<int>((colorRatio - 1.0) * 2000);
    } else if (colorRatio < 0.9) {
        // Too much red - decrease temperature (cooler)
        colorTemp = 6500 - static_cast<int>((1.0 - colorRatio) * 2000);
    }
    
    // Clamp to reasonable range
    colorTemp = qBound(2500, colorTemp, 9000);
    
    qDebug() << "CameraService::autoWhiteBalance - setting color temperature to" << colorTemp << "K";
    
    // Apply color temperature adjustment
    if (_camera->isWhiteBalanceModeSupported(QCamera::WhiteBalanceManual)) {
        _camera->setWhiteBalanceMode(QCamera::WhiteBalanceManual);
        _camera->setColorTemperature(colorTemp);
    } else {
        qDebug() << "CameraService::autoWhiteBalance - manual white balance not supported, using auto mode";
        _camera->setWhiteBalanceMode(QCamera::WhiteBalanceAuto);
    }
}

