#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QMap>
#include <QVariant>
#include <QTimer>
#include <QFuture>
#include <QFutureWatcher>
#include <gphoto2/gphoto2.h>

#ifdef Q_OS_ANDROID
#include "AndroidUsbHelper.h"
#endif

struct PTPCameraInfo {
    QString id;
    QString manufacturer;
    QString model;
    QString port;
};

class PTPCameraService : public QObject
{
    Q_OBJECT

public:
    explicit PTPCameraService(QObject *parent = nullptr);
    ~PTPCameraService();

    // Camera detection and connection
    QList<PTPCameraInfo> detectCameras();
    bool connect(const PTPCameraInfo& cameraInfo, bool silent = false);
    void disconnect();
    bool isConnected() const { return _camera != nullptr; }

    // Camera capabilities
    QMap<QString, QVariant> getCapabilities();
    
    // Live view
    QImage getLiveViewFrame();
    
    // Settings control
    bool setSetting(const QString& name, const QVariant& value);
    QVariant getSetting(const QString& name);
    
    // Flip transformations
    void setFlipHorizontal(bool enabled) { _flipHorizontal = enabled; }
    void setFlipVertical(bool enabled) { _flipVertical = enabled; }
    
    // Capture - always captures to SD card and transfers via USB
    QString captureImage();
    
    // Video recording
    bool startRecording();
    bool stopRecording();
    bool isRecording() const { return _isRecording; }
    
    // Battery monitoring
    int getBatteryLevel();  // Returns 0-100, or -1 if unavailable
    
    // Hot-plug detection
    void startHotplugMonitoring();
    void stopHotplugMonitoring();

signals:
    void frameReady(const QImage& frame);
    void cameraConnected(const PTPCameraInfo& info);
    void cameraDisconnected();
    void captureComplete(const QString& filePath);
    void error(const QString& message);
    void warning(const QString& message);  // For non-critical issues
    
    // Hot-plug signals
    void cameraPlugged(const PTPCameraInfo& info);
    void cameraUnplugged(const QString& cameraId);

private slots:
    void checkCameraConnection();
    void captureLiveViewFrame();
    void onCameraInitFinished();

private:
    GPContext* _context;
    Camera* _camera;
    bool _isRecording;
    bool _silentMode;  // Suppress error messages during auto-reconnect
    bool _flipHorizontal = false;
    bool _flipVertical = false;
    QImage _lastProcessedFrame;  // Store last processed frame for snapshots
    QTimer* _hotplugTimer;
    QTimer* _liveViewTimer;
    QList<PTPCameraInfo> _lastDetectedCameras;
    PTPCameraInfo _pendingConnection;  // Camera info for async connection
    QFutureWatcher<int>* _initWatcher;  // Watcher for async camera init
    
    // Cache of widget names for manufacturer-specific properties
    QString _exposureModeWidgetName;
    QString _shutterSpeedWidgetName;
    QString _whiteBalanceWidgetName;
    
#ifdef Q_OS_ANDROID
    int _androidUsbFd;  // Android USB file descriptor
    QString _androidDeviceName;  // Android USB device name
#endif
    
    // Helper methods
    bool checkError(int result, const QString& operation);
    CameraWidget* findWidget(const QString& name);
    QString getCameraManufacturer();
    QString getCameraModel();
    QString getCurrentCameraPort() const;
    void embedExifMetadata(const QString& filePath);
    
    // Helper functions to convert numeric PTP values to readable strings
    QString convertShutterSpeedToReadable(const QString& rawValue);
    QString convertWhiteBalanceToReadable(const QString& rawValue);
    QString convertReadableToShutterSpeed(const QString& readable);
    QString convertReadableToWhiteBalance(const QString& readable);
};
