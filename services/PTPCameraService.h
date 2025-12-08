#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QImage>
#include <QMap>
#include <QVariant>
#include <gphoto2/gphoto2.h>

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
    bool connect(const PTPCameraInfo& cameraInfo);
    void disconnect();
    bool isConnected() const { return _camera != nullptr; }

    // Camera capabilities
    QMap<QString, QVariant> getCapabilities();
    
    // Live view
    QImage getLiveViewFrame();
    
    // Settings control
    bool setSetting(const QString& name, const QVariant& value);
    QVariant getSetting(const QString& name);
    
    // Capture - always captures to SD card and transfers via USB
    QString captureImage();
    
    // Video recording
    bool startRecording();
    bool stopRecording();
    bool isRecording() const { return _isRecording; }

signals:
    void frameReady(const QImage& frame);
    void cameraConnected(const PTPCameraInfo& info);
    void cameraDisconnected();
    void captureComplete(const QString& filePath);
    void error(const QString& message);

private:
    GPContext* _context;
    Camera* _camera;
    bool _isRecording;
    
    // Helper methods
    bool checkError(int result, const QString& operation);
    CameraWidget* findWidget(const QString& name);
    QString getCameraManufacturer();
    QString getCameraModel();
};
