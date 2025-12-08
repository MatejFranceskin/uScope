#ifndef CAMERASERVICE_H
#define CAMERASERVICE_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QVideoFrame>
#include <QList>
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
};

#endif // CAMERASERVICE_H
