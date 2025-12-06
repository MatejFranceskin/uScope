#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QVideoFrame>
#include <QList>
#include "../models/CameraProfile.h"
#include "../models/CapturedImage.h"

class CameraService;

/**
 * Camera business logic controller
 * Coordinates camera operations and manages capture workflow
 */
class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(QObject* parent = nullptr);
    ~CameraController();

    // Camera discovery
    QList<CameraProfile> availableCameras();
    
    // Camera lifecycle
    bool isActive() const;
    QString currentCameraId() const;

public slots:
    void startCamera(const QString& cameraId);
    void stopCamera();
    void captureImage();
    void autoStartCamera();  // Auto-start first available camera

signals:
    void frameReady(const QVideoFrame& frame);
    void cameraConnected(const QString& cameraId, const QString& name);
    void cameraDisconnected(const QString& cameraId);
    void imageCaptured(const CapturedImage& image);
    void error(const QString& message);

private slots:
    void onFrameReady(const QVideoFrame& frame);
    void onCameraConnected(const QString& cameraId, const QString& name);
    void onCameraDisconnected(const QString& cameraId);
    void onServiceError(const QString& message);

private:
    CameraService* _service;
};

#endif // CAMERACONTROLLER_H
