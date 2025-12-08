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
        emit frameReady(frame);
    }
}

void CameraService::onCameraErrorOccurred(QCamera::Error error, const QString& errorString)
{
    Q_UNUSED(error);
    emit this->error(QString("Camera error: %1").arg(errorString));
}
