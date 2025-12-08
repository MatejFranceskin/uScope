#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QVideoFrame>
#include <QList>
#include <QCameraFormat>
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
    QList<QCameraFormat> availableFormats(const QString& cameraId);
    QCameraFormat currentFormat() const;
    
    // Camera lifecycle
    bool isActive() const;
    QString currentCameraId() const;
    
    // Settings persistence
    void saveCurrentCamera();
    void saveCameraSelection(const QString& cameraId, const QCameraFormat& format);
    void restoreLastCamera();
    
    // Camera monitoring for reconnection
    void startCameraMonitoring();
    void stopCameraMonitoring();

public slots:
    void startCamera(const QString& cameraId);
    void startCamera(const QString& cameraId, const QCameraFormat& format);
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
    void checkForLastCamera();  // Timer callback to check for last camera

private:
    CameraService* _service;
    QString _lastCameraName;  // Last successfully connected camera name
    QSize _lastResolution;  // Last used resolution
    qreal _lastFrameRate;   // Last used frame rate
    QTimer* _monitorTimer;  // Timer for camera reconnection monitoring
};

#endif // CAMERACONTROLLER_H
