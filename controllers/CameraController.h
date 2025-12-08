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
    void saveCameraControls(const QString& cameraId);
    void restoreCameraControls(const QString& cameraId);
    
    // Camera control getters (for UI restoration)
    qreal exposure() const { return _exposure; }
    int brightness() const { return _brightness; }
    int contrast() const { return _contrast; }
    int saturation() const { return _saturation; }
    bool flipHorizontal() const { return _flipHorizontal; }
    bool flipVertical() const { return _flipVertical; }
    int whiteBalance() const { return _whiteBalance; }
    
    // Camera monitoring for reconnection
    void startCameraMonitoring();
    void stopCameraMonitoring();

public slots:
    void startCamera(const QString& cameraId);
    void startCamera(const QString& cameraId, const QCameraFormat& format);
    void stopCamera();
    void captureImage();
    void autoStartCamera();  // Auto-start first available camera
    
    // Camera controls (US3)
    void setExposure(qreal value);
    void setWhiteBalance(int mode);  // QCamera::WhiteBalanceMode as int
    void setBrightness(int value);
    void setContrast(int value);
    void setSaturation(int value);
    void setFlipHorizontal(bool enabled);
    void setFlipVertical(bool enabled);
    void autoWhiteBalance();  // Auto white balance using current frame

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
    
    // Camera control values (for persistence)
    qreal _exposure;        // Exposure time in ms (10-1000)
    int _brightness;        // Brightness (-100 to 100)
    int _contrast;          // Contrast (-100 to 100)
    int _saturation;        // Saturation (-100 to 100)
    bool _flipHorizontal;   // Flip horizontal
    bool _flipVertical;     // Flip vertical
    int _whiteBalance;      // QCamera::WhiteBalanceMode as int
};

#endif // CAMERACONTROLLER_H
