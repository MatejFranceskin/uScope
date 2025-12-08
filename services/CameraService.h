#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QList>
#include <QElapsedTimer>
#include "../models/CameraProfile.h"

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
    QList<QCameraFormat> availableFormats(const QString& cameraId);
    QCameraFormat currentFormat() const;

    // Camera lifecycle
    bool startCamera(const QString& cameraId);
    bool startCamera(const QString& cameraId, const QCameraFormat& format);
    void stopCamera();
    bool isActive() const;

    // Frame capture
    QImage captureFrame();
    QString currentCameraId() const { return _currentCameraId; }
    
    // Camera controls (US3)
    void setExposure(qreal value);           // Manual exposure time
    void setWhiteBalance(QCamera::WhiteBalanceMode mode);
    void setBrightness(int value);           // -100 to 100
    void setContrast(int value);             // -100 to 100
    void setSaturation(int value);           // -100 to 100
    void setFlipHorizontal(bool enabled);
    void setFlipVertical(bool enabled);
    void autoWhiteBalance();  // Auto white balance using current frame

signals:
    void frameReady(const QVideoFrame& frame);
    void cameraConnected(const QString& cameraId, const QString& name);
    void cameraDisconnected(const QString& cameraId);
    void error(const QString& message);

private slots:
    void onVideoFrameChanged(const QVideoFrame& frame);
    void onCameraErrorOccurred(QCamera::Error error, const QString& errorString);

private:
    QCamera* _camera;
    QMediaCaptureSession* _captureSession;
    QVideoSink* _videoSink;
    QString _currentCameraId;
    QVideoFrame _lastFrame;
    
    // Latency measurement for SC-004 verification
    QElapsedTimer _controlChangeTimer;
    QString _lastControlChange;
};

#endif // CAMERASERVICE_H
