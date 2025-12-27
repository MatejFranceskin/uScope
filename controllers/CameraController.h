#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <QObject>
#include <QVideoFrame>
#include <QList>
#include <QSize>
#include "../models/CameraProfile.h"
#include "../models/CapturedImage.h"

class CameraService;
#if !defined(Q_OS_IOS)
class PTPCameraService;
#endif

enum class CameraType {
    UVC,    // USB Video Class (V4L2)
    PTP     // Picture Transfer Protocol (DSLR/Mirrorless)
};

/**
 * Camera business logic controller
 * Coordinates camera operations and manages capture workflow
 * Supports both UVC (webcam/microscope) and PTP (DSLR) cameras
 */
class CameraController : public QObject
{
    Q_OBJECT

public:
    explicit CameraController(QObject* parent = nullptr);
    ~CameraController();

    // Camera discovery
    QList<CameraProfile> availableCameras();
    QList<QSize> availableResolutions(const QString& cameraId);
    QList<double> availableFrameRates(const QString& cameraId, const QSize& resolution);
    QSize currentResolution() const;
    double currentFrameRate() const;
    
    // Camera lifecycle
    bool isActive() const;
    QString currentCameraId() const;
    CameraType currentCameraType() const { return _currentCameraType; }
#if !defined(Q_OS_IOS)
    bool isPTPCamera() const { return _currentCameraType == CameraType::PTP; }
#else
    bool isPTPCamera() const { return false; }
#endif
    
    // Settings persistence
    void saveCurrentCamera();
    void saveCameraSelection(const QString& cameraId, const QSize& resolution);
    void restoreLastCamera();
    void saveCameraControls(const QString& cameraId);
    void restoreCameraControls(const QString& cameraId);
    QSize getSavedResolution(const QString& cameraId) const;
    
    // Camera control getters (for UI restoration)
    qreal exposure() const { return _exposure; }
    int brightness() const { return _brightness; }
    int contrast() const { return _contrast; }
    int saturation() const { return _saturation; }
    bool flipHorizontal() const { return _flipHorizontal; }
    bool flipVertical() const { return _flipVertical; }
    int whiteBalance() const { return _whiteBalance; }
    bool autoExposure() const { return _autoExposure; }
    bool autoWhiteBalance() const { return _autoWhiteBalance; }
    
    // Read current values from camera hardware
    int getCurrentExposure() const;
    int getCurrentWhiteBalance() const;
    
    // Camera monitoring for reconnection
    void startCameraMonitoring();
    void stopCameraMonitoring();
    
    // PTP camera capabilities and settings
    QMap<QString, QVariant> getPTPCapabilities() const;
    void setPTPSetting(const QString& name, const QString& value);

public slots:
    void startCamera(const QString& cameraId);
    void startCamera(const QString& cameraId, const QSize& resolution, double frameRate = 30.0);
    void stopCamera();
    void captureImage();
    void autoStartCamera();  // Auto-start first available camera
    
    // Camera controls (US3) - OpenCV-based hardware control
    void setExposure(int value);         // -13 to -1 (log2 exposure)
    void setGain(int value);             // 0-100
    void setWhiteBalance(int value);     // Color temperature 2800-6500K
    void setBrightness(int value);       // 0-255
    void setContrast(int value);         // 0-255
    void setSaturation(int value);       // 0-255
    void setFlipHorizontal(bool enabled);
    void setFlipVertical(bool enabled);
    void setAutoWhiteBalance(bool enabled);  // Toggle auto WB
    void setAutoExposure(bool enabled);      // Toggle auto exposure

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
#if !defined(Q_OS_IOS)
    PTPCameraService* _ptpService;
#endif
    CameraType _currentCameraType;
    QString _currentCameraId;  // ID of currently active camera
    QString _lastCameraName;  // Last successfully connected camera name
    QTimer* _monitorTimer;    // Timer for camera reconnection monitoring
    
    // Camera control values (for persistence)
    qreal _exposure;        // Exposure time in ms (10-1000)
    int _brightness;        // Brightness (-100 to 100)
    int _contrast;          // Contrast (-100 to 100)
    int _saturation;        // Saturation (-100 to 100)
    bool _flipHorizontal;   // Flip horizontal
    bool _flipVertical;     // Flip vertical
    int _whiteBalance;      // QCamera::WhiteBalanceMode as int
    bool _autoExposure;     // Auto exposure enabled
    bool _autoWhiteBalance; // Auto white balance enabled
    
    // Helper methods
    CameraType detectCameraType(const QString& cameraId) const;
    void cleanupServices();
};

#endif // CAMERACONTROLLER_H
