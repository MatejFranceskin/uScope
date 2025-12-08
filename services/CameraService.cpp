#include "CameraService.h"
#include "CaptureThread.h"
#include <QMediaDevices>
#include <QCameraDevice>
#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

CameraService::CameraService(QObject* parent)
    : QObject(parent)
    , _videoSink(nullptr)
    , _captureThread(nullptr)
{
    _videoSink = new QVideoSink(this);
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

QList<QSize> CameraService::availableResolutions(const QString& cameraId)
{
    QList<QSize> resolutions;
    QSet<QSize> uniqueResolutions;  // To avoid duplicates
    
    // Find the camera device
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            // Get all video formats
            const QList<QCameraFormat> formats = device.videoFormats();
            for (const QCameraFormat& format : formats) {
                QSize resolution = format.resolution();
                if (resolution.isValid() && !uniqueResolutions.contains(resolution)) {
                    uniqueResolutions.insert(resolution);
                    resolutions.append(resolution);
                }
            }
            break;
        }
    }
    
    // If no formats found, provide common fallback resolutions
    if (resolutions.isEmpty()) {
        resolutions << QSize(640, 480)
                    << QSize(800, 600)
                    << QSize(1280, 720)
                    << QSize(1920, 1080);
    }
    
    return resolutions;
}

QList<double> CameraService::availableFrameRates(const QString& cameraId, const QSize& resolution)
{
    QList<double> frameRates;
    QSet<double> uniqueRates;  // To avoid duplicates
    
    // Find the camera device
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            // Get all video formats matching this resolution
            const QList<QCameraFormat> formats = device.videoFormats();
            for (const QCameraFormat& format : formats) {
                if (format.resolution() == resolution) {
                    // We use MJPEG in OpenCV (fourcc 'MJPG'), so filter for Motion-JPEG formats
                    // Qt's QVideoFrameFormat::Format_Jpeg corresponds to MJPEG
                    QVideoFrameFormat::PixelFormat pixelFormat = format.pixelFormat();
                    
                    qDebug() << "CameraService::availableFrameRates - resolution:" << resolution 
                             << "pixelFormat:" << pixelFormat 
                             << "fps:" << format.minFrameRate() << "-" << format.maxFrameRate();
                    
                    // Only include MJPEG/JPEG formats since that's what we configure in startCamera
                    if (pixelFormat == QVideoFrameFormat::Format_Jpeg) {
                        double minFps = format.minFrameRate();
                        double maxFps = format.maxFrameRate();
                        
                        // Add the max frame rate (most useful)
                        if (maxFps > 0 && !uniqueRates.contains(maxFps)) {
                            uniqueRates.insert(maxFps);
                            frameRates.append(maxFps);
                        }
                        
                        // Also add min if it's different
                        if (minFps > 0 && minFps != maxFps && !uniqueRates.contains(minFps)) {
                            uniqueRates.insert(minFps);
                            frameRates.append(minFps);
                        }
                    }
                }
            }
            break;
        }
    }
    
    // Sort in descending order (highest FPS first)
    std::sort(frameRates.begin(), frameRates.end(), std::greater<double>());
    
    // If no MJPEG frame rates found, fall back to 30fps (most common for MJPEG)
    if (frameRates.isEmpty()) {
        frameRates << 30.0;
    }
    
    return frameRates;
}

QSize CameraService::currentResolution() const
{
    return _currentResolution;
}

double CameraService::currentFrameRate() const
{
    return _currentFrameRate;
}

bool CameraService::startCamera(const QString& cameraId)
{
    return startCamera(cameraId, QSize(1280, 720), 30.0);  // Default to 720p @ 30fps
}

bool CameraService::startCamera(const QString& cameraId, const QSize& resolution, double frameRate)
{
    // Check if we're restarting the same camera (just changing resolution/fps)
    bool isRestart = (_currentCameraId == cameraId && _cvCapture.isOpened());
    
    // Stop current camera silently (don't emit disconnect since we're starting another)
    // User is intentionally switching cameras, not experiencing a disconnect
    if (_captureThread && _captureThread->isRunning()) {
        _captureThread->stop();
        _captureThread->wait(1000);
        delete _captureThread;
        _captureThread = nullptr;
    }
    if (_cvCapture.isOpened()) {
        _cvCapture.release();
        // Don't emit disconnect - we're switching cameras intentionally
        _currentCameraId.clear();
    }
    
    qDebug() << "CameraService::startCamera - requested resolution:" << resolution << "@" << frameRate << "fps";
    
    // Extract camera index from device ID (e.g., "/dev/video0" -> 0)
    int cameraIndex = getCameraIndex(cameraId);
    if (cameraIndex < 0) {
        emit error(QString("Invalid camera ID: %1").arg(cameraId));
        return false;
    }
    
    // Open camera with OpenCV using V4L2 backend directly
    if (!_cvCapture.open(cameraIndex, cv::CAP_V4L2)) {
        emit error(QString("Failed to open camera index %1").arg(cameraIndex));
        return false;
    }
    
    // Set MJPEG format for 30fps (YUYV is limited to 10fps at 720p)
    _cvCapture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    
    // Request RGB format directly from decoder (avoid BGR->RGB conversion)
    _cvCapture.set(cv::CAP_PROP_CONVERT_RGB, 1);
    
    // Set resolution
    _cvCapture.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width());
    _cvCapture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height());
    
    // Set requested frame rate
    _cvCapture.set(cv::CAP_PROP_FPS, frameRate);
    
    // Try different auto exposure settings
    // 1 = manual mode, 3 = aperture priority mode
    qDebug() << "CameraService::startCamera - current auto_exposure:" << _cvCapture.get(cv::CAP_PROP_AUTO_EXPOSURE);
    _cvCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, 3);  // Try aperture priority
    qDebug() << "CameraService::startCamera - auto_exposure after setting to 3:" << _cvCapture.get(cv::CAP_PROP_AUTO_EXPOSURE);
    
    // Try to read a test frame to verify camera is working
    cv::Mat testFrame;
    if (!_cvCapture.read(testFrame) || testFrame.empty()) {
        _cvCapture.release();
        emit error(QString("Camera opened but cannot read frames from index %1").arg(cameraIndex));
        return false;
    }
    
    // Read back actual resolution and fps
    double actualWidth = _cvCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    double actualHeight = _cvCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
    double actualFps = _cvCapture.get(cv::CAP_PROP_FPS);
    
    _currentResolution = QSize(static_cast<int>(actualWidth), static_cast<int>(actualHeight));
    _currentFrameRate = actualFps;
    _currentCameraId = cameraId;
    
    qDebug() << "CameraService::startCamera - actual resolution:" << _currentResolution << "@" << actualFps << "fps";
    qDebug() << "CameraService::startCamera - format:" << _cvCapture.get(cv::CAP_PROP_FOURCC);
    qDebug() << "CameraService::startCamera - test frame channels:" << testFrame.channels() << "type:" << testFrame.type();
    
    // Set all controls to automatic for best default image quality
    _cvCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, 3);  // Aperture priority mode
    _cvCapture.set(cv::CAP_PROP_AUTO_WB, 1);        // Auto white balance
    _cvCapture.set(cv::CAP_PROP_AUTOFOCUS, 1);      // Auto focus (if supported)
    
    // Set default values for manual controls
    _cvCapture.set(cv::CAP_PROP_BRIGHTNESS, 128);   // Default brightness
    _cvCapture.set(cv::CAP_PROP_CONTRAST, 32);      // Default contrast  
    _cvCapture.set(cv::CAP_PROP_SATURATION, 64);    // Default saturation
    
    qDebug() << "CameraService::startCamera - auto_exposure:" << _cvCapture.get(cv::CAP_PROP_AUTO_EXPOSURE);
    qDebug() << "CameraService::startCamera - auto_wb:" << _cvCapture.get(cv::CAP_PROP_AUTO_WB);
    qDebug() << "CameraService::startCamera - brightness:" << _cvCapture.get(cv::CAP_PROP_BRIGHTNESS);
    qDebug() << "CameraService::startCamera - contrast:" << _cvCapture.get(cv::CAP_PROP_CONTRAST);
    qDebug() << "CameraService::startCamera - saturation:" << _cvCapture.get(cv::CAP_PROP_SATURATION);
    
    // Process the test frame to display it immediately
    processFrame(testFrame);
    
    // Start capture thread for continuous frame reading
    _captureThread = new CaptureThread(&_cvCapture, this);
    connect(_captureThread, &CaptureThread::frameCaptured, 
            this, &CameraService::onFrameCaptured);
    _captureThread->start();
    
    // Find camera name for signal
    QString cameraName = cameraId;
    const QList<QCameraDevice> devices = QMediaDevices::videoInputs();
    for (const QCameraDevice& device : devices) {
        if (device.id() == cameraId.toUtf8()) {
            cameraName = device.description();
            break;
        }
    }
    
    // Only emit connected signal if this is not a restart
    if (!isRestart) {
        emit cameraConnected(cameraId, cameraName);
    }
    return true;
}

void CameraService::stopCamera()
{
    if (_captureThread && _captureThread->isRunning()) {
        _captureThread->stop();
        _captureThread->wait(1000);  // Wait up to 1 second
        delete _captureThread;
        _captureThread = nullptr;
    }
    
    if (_cvCapture.isOpened()) {
        _cvCapture.release();
        
        if (!_currentCameraId.isEmpty()) {
            emit cameraDisconnected(_currentCameraId);
            _currentCameraId.clear();
        }
    }
}

bool CameraService::isActive() const
{
    return _cvCapture.isOpened() && _captureThread && _captureThread->isRunning();
}

QImage CameraService::captureFrame()
{
    if (!_cvCapture.isOpened()) {
        return QImage();
    }
    
    cv::Mat frame;
    if (_cvCapture.read(frame) && !frame.empty()) {
        // Convert BGR to RGB
        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
        
        // Convert to QImage
        QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, 
                     rgbFrame.step, QImage::Format_RGB888);
        return image.copy();  // Deep copy to avoid data invalidation
    }
    
    return QImage();
}

// Slot called when capture thread emits a new frame
void CameraService::onFrameCaptured(const cv::Mat& frame)
{
    processFrame(frame);
}

// T105c1: Process frame from any capture source (OpenCV, libgphoto2, etc.)
void CameraService::processFrame(const cv::Mat& frame)
{
    cv::Mat processedFrame = frame.clone();
    
    // Apply flip transformations if needed
    if (_flipHorizontal && _flipVertical) {
        cv::flip(processedFrame, processedFrame, -1);  // Both axes
    } else if (_flipHorizontal) {
        cv::flip(processedFrame, processedFrame, 1);   // Horizontal
    } else if (_flipVertical) {
        cv::flip(processedFrame, processedFrame, 0);   // Vertical
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    // Measure latency for SC-004 verification (<200ms requirement)
    if (_controlChangeTimer.isValid() && !_lastControlChange.isEmpty()) {
        qint64 latencyMs = _controlChangeTimer.elapsed();
        qDebug() << "CameraService::processFrame - control change latency:"
                 << _lastControlChange << "took" << latencyMs << "ms"
                 << (latencyMs < 200 ? "✓ PASS" : "✗ FAIL");
        _lastControlChange.clear();
    }
#endif
    
    // T105d: Convert to QVideoFrame and emit
    QVideoFrame videoFrame = cvMatToQVideoFrame(processedFrame);
    if (videoFrame.isValid()) {
        emit frameReady(videoFrame);
    }
}

// T105d: Convert cv::Mat to QVideoFrame via QImage intermediate
QVideoFrame CameraService::cvMatToQVideoFrame(const cv::Mat& mat)
{
    static bool firstFrame = true;
    if (firstFrame) {
        qDebug() << "CameraService::cvMatToQVideoFrame - first frame: channels=" << mat.channels() 
                 << "type=" << mat.type() << "size=" << mat.cols << "x" << mat.rows;
        firstFrame = false;
    }
    
    cv::Mat rgbFrame;
    
    // CAP_PROP_CONVERT_RGB doesn't always work with V4L2
    // OpenCV V4L2 backend typically outputs BGR, so always convert
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbFrame, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgbFrame, cv::COLOR_GRAY2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgbFrame, cv::COLOR_BGRA2RGB);
    } else {
        rgbFrame = mat;
    }
    
    // Create QImage from cv::Mat data (should be RGB at this point)
    QImage image(rgbFrame.data, rgbFrame.cols, rgbFrame.rows,
                 static_cast<int>(rgbFrame.step), QImage::Format_RGB888);
    
    // Deep copy to avoid data invalidation when cv::Mat goes out of scope
    QImage imageCopy = image.copy();
    
    // Create QVideoFrame from QImage
    // Use QVideoFrameFormat constructor for Qt 6 compatibility
    QVideoFrameFormat format(imageCopy.size(), QVideoFrameFormat::Format_RGBX8888);
    QVideoFrame frame(format);
    
    if (frame.map(QVideoFrame::WriteOnly)) {
        // Convert QImage to RGBX8888 and copy to frame buffer
        QImage rgbxImage = imageCopy.convertToFormat(QImage::Format_RGBX8888);
        memcpy(frame.bits(0), rgbxImage.constBits(), rgbxImage.sizeInBytes());
        frame.unmap();
    }
    
    return frame;
}

// Helper to extract camera index from device ID (e.g., "/dev/video0" -> 0)
int CameraService::getCameraIndex(const QString& cameraId)
{
    // Extract number from "/dev/videoX" format
    if (cameraId.startsWith("/dev/video")) {
        bool ok;
        int index = cameraId.mid(10).toInt(&ok);  // Skip "/dev/video"
        if (ok) {
            return index;
        }
    }
    return -1;
}

// Camera controls (US3) - Using OpenCV VideoCapture for hardware control
void CameraService::setExposure(int value)
{
    if (!_cvCapture.isOpened()) {
        qDebug() << "CameraService::setExposure - camera not open";
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("exposure=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // Disable auto exposure first (set to manual mode)
    _cvCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);  // 1 = manual mode
    
    // Set exposure (typically -13 to -1, where -1 is longest exposure)
    if (_cvCapture.set(cv::CAP_PROP_EXPOSURE, value)) {
        qDebug() << "CameraService::setExposure - set to" << value;
        double readback = _cvCapture.get(cv::CAP_PROP_EXPOSURE);
        qDebug() << "CameraService::setExposure - readback:" << readback;
    } else {
        qDebug() << "CameraService::setExposure - failed to set";
    }
}

void CameraService::setGain(int value)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
    if (_cvCapture.set(cv::CAP_PROP_GAIN, value)) {
        qDebug() << "CameraService::setGain - set to" << value;
    } else {
        qDebug() << "CameraService::setGain - failed to set";
    }
}

void CameraService::setWhiteBalance(int value)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
    // Disable auto white balance first
    _cvCapture.set(cv::CAP_PROP_AUTO_WB, 0);
    
    // Set white balance temperature (2800-6500K typically)
    if (_cvCapture.set(cv::CAP_PROP_WB_TEMPERATURE, value)) {
        qDebug() << "CameraService::setWhiteBalance - set to" << value << "K";
    } else {
        qDebug() << "CameraService::setWhiteBalance - failed to set";
    }
}

void CameraService::setBrightness(int value)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("brightness=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // OpenCV brightness is 0-255
    if (_cvCapture.set(cv::CAP_PROP_BRIGHTNESS, value)) {
        qDebug() << "CameraService::setBrightness - set to" << value;
    } else {
        qDebug() << "CameraService::setBrightness - failed to set";
    }
}

void CameraService::setContrast(int value)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("contrast=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // OpenCV contrast is 0-255
    if (_cvCapture.set(cv::CAP_PROP_CONTRAST, value)) {
        qDebug() << "CameraService::setContrast - set to" << value;
    } else {
        qDebug() << "CameraService::setContrast - failed to set";
    }
}

void CameraService::setSaturation(int value)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    _lastControlChange = QString("saturation=%1").arg(value);
    _controlChangeTimer.start();
#endif
    
    // OpenCV saturation is 0-255
    if (_cvCapture.set(cv::CAP_PROP_SATURATION, value)) {
        qDebug() << "CameraService::setSaturation - set to" << value;
    } else {
        qDebug() << "CameraService::setSaturation - failed to set";
    }
}

void CameraService::setFlipHorizontal(bool enabled)
{
    _flipHorizontal = enabled;
    qDebug() << "CameraService::setFlipHorizontal - set to" << enabled;
}

void CameraService::setFlipVertical(bool enabled)
{
    _flipVertical = enabled;
    qDebug() << "CameraService::setFlipVertical - set to" << enabled;
}

void CameraService::setAutoWhiteBalance(bool enabled)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
    if (_cvCapture.set(cv::CAP_PROP_AUTO_WB, enabled ? 1 : 0)) {
        qDebug() << "CameraService::setAutoWhiteBalance - set to" << (enabled ? "ON" : "OFF");
    } else {
        qDebug() << "CameraService::setAutoWhiteBalance - failed to set";
    }
}

void CameraService::setAutoExposure(bool enabled)
{
    if (!_cvCapture.isOpened()) {
        return;
    }
    
    // CAP_PROP_AUTO_EXPOSURE for V4L2: 1 = manual mode, 3 = aperture priority (auto)
    int mode = enabled ? 3 : 1;
    if (_cvCapture.set(cv::CAP_PROP_AUTO_EXPOSURE, mode)) {
        qDebug() << "CameraService::setAutoExposure - set to" << (enabled ? "AUTO (3)" : "MANUAL (1)");
    } else {
        qDebug() << "CameraService::setAutoExposure - failed to set";
    }
}

int CameraService::getCurrentExposure() const
{
    if (!_cvCapture.isOpened()) {
        qDebug() << "CameraService::getCurrentExposure - camera not open, returning 100";
        return 100;  // Default value
    }
    
    double exposure = _cvCapture.get(cv::CAP_PROP_EXPOSURE);
    qDebug() << "CameraService::getCurrentExposure - read value:" << exposure;
    return static_cast<int>(exposure);
}

int CameraService::getCurrentWhiteBalance() const
{
    if (!_cvCapture.isOpened()) {
        qDebug() << "CameraService::getCurrentWhiteBalance - camera not open, returning 4600";
        return 4600;  // Default value
    }
    
    double wb = _cvCapture.get(cv::CAP_PROP_WB_TEMPERATURE);
    qDebug() << "CameraService::getCurrentWhiteBalance - read value:" << wb;
    return static_cast<int>(wb);
}
