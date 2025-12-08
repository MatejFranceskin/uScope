#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <QObject>
#include <QVideoSink>
#include <QVideoFrame>
#include <QImage>
#include <QList>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "../models/CameraProfile.h"
#include "CaptureThread.h"

#ifdef ENABLE_LATENCY_MEASUREMENT
#include <QElapsedTimer>
#endif

/**
 * Camera backend service managing QCamera lifecycle
 * Handles camera enumeration, video streaming, and frame capture
 */
class CameraService : public QObject
{
    Q_OBJECT

public:
    explicit CameraService(QObject* parent = nullptr);
    ~CameraService();

    // Camera enumeration
    QList<CameraProfile> enumerateCameras();
    QList<QSize> availableResolutions(const QString& cameraId);
    QList<double> availableFrameRates(const QString& cameraId, const QSize& resolution);
    QSize currentResolution() const;
    double currentFrameRate() const;

    // Camera lifecycle
    bool startCamera(const QString& cameraId);
    bool startCamera(const QString& cameraId, const QSize& resolution, double frameRate = 30.0);
    void stopCamera();
    bool isActive() const;
    
    // Frame processing (T105c1) - accepts frames from any source
    void processFrame(const cv::Mat& frame);

    // Frame capture
    QImage captureFrame();
    QString currentCameraId() const { return _currentCameraId; }
    
    // Camera controls (US3) - via OpenCV VideoCapture for hardware control
    void setExposure(int value);             // -13 to -1 (log2 of exposure time)
    void setGain(int value);                 // 0-100
    void setWhiteBalance(int value);         // Color temperature 2800-6500K  
    void setBrightness(int value);           // 0-255
    void setContrast(int value);             // 0-255
    void setSaturation(int value);           // 0-255
    void setFlipHorizontal(bool enabled);
    void setFlipVertical(bool enabled);
    void setAutoWhiteBalance(bool enabled);  // Enable/disable auto WB
    void setAutoExposure(bool enabled);      // Enable/disable auto exposure
    
    // Read current values from camera hardware
    int getCurrentExposure() const;          // Read actual exposure from camera
    int getCurrentWhiteBalance() const;      // Read actual WB temperature from camera

signals:
    void frameReady(const QVideoFrame& frame);
    void cameraConnected(const QString& cameraId, const QString& name);
    void cameraDisconnected(const QString& cameraId);
    void error(const QString& message);
    void stopCaptureThread();  // Internal signal to stop thread

private slots:
    void onFrameCaptured(const cv::Mat& frame);

private:
    QVideoSink* _videoSink;
    QString _currentCameraId;
    QSize _currentResolution;
    double _currentFrameRate = 30.0;
    
    // OpenCV VideoCapture - primary capture mechanism
    cv::VideoCapture _cvCapture;
    CaptureThread* _captureThread;
    
    // Image processing state
    bool _flipHorizontal = false;
    bool _flipVertical = false;
    
    // Helper methods
    int getCameraIndex(const QString& cameraId);
    QVideoFrame cvMatToQVideoFrame(const cv::Mat& mat);
    
#ifdef ENABLE_LATENCY_MEASUREMENT
    // Latency measurement for SC-004 verification (development/testing only)
    QElapsedTimer _controlChangeTimer;
    QString _lastControlChange;
#endif
};

#endif // CAMERASERVICE_H
