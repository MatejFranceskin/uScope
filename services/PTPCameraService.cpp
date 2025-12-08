#include "PTPCameraService.h"
#include <QDebug>
#include <QBuffer>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>

PTPCameraService::PTPCameraService(QObject *parent)
    : QObject(parent)
    , _context(nullptr)
    , _camera(nullptr)
    , _isRecording(false)
{
    _context = gp_context_new();
    
    // Create captures directory if it doesn't exist
    QString capturesPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/uScope/captures";
    QDir dir;
    if (!dir.exists(capturesPath)) {
        dir.mkpath(capturesPath);
        qDebug() << "Created captures directory:" << capturesPath;
    }
}

PTPCameraService::~PTPCameraService()
{
    disconnect();
    
    if (_context) {
        gp_context_unref(_context);
        _context = nullptr;
    }
}

QList<PTPCameraInfo> PTPCameraService::detectCameras()
{
    QList<PTPCameraInfo> cameras;
    
    CameraList *list;
    int ret = gp_list_new(&list);
    if (ret != GP_OK) {
        qWarning() << "Failed to create camera list:" << gp_result_as_string(ret);
        return cameras;
    }
    
    ret = gp_camera_autodetect(list, _context);
    if (ret != GP_OK) {
        qWarning() << "Failed to autodetect cameras:" << gp_result_as_string(ret);
        gp_list_free(list);
        return cameras;
    }
    
    int count = gp_list_count(list);
    for (int i = 0; i < count; i++) {
        const char *name = nullptr;
        const char *port = nullptr;
        
        gp_list_get_name(list, i, &name);
        gp_list_get_value(list, i, &port);
        
        if (name && port) {
            PTPCameraInfo info;
            info.id = QString("ptp://%1").arg(port);
            info.model = QString::fromUtf8(name);
            info.port = QString::fromUtf8(port);
            
            // Parse manufacturer from model name (typically "Manufacturer Model")
            QStringList parts = info.model.split(' ', Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                info.manufacturer = parts.first();
            }
            
            cameras.append(info);
        }
    }
    
    gp_list_free(list);
    return cameras;
}

bool PTPCameraService::connect(const PTPCameraInfo& cameraInfo)
{
    if (_camera) {
        disconnect();
    }
    
    int ret = gp_camera_new(&_camera);
    if (!checkError(ret, "create camera")) {
        return false;
    }
    
    // Set the camera port
    GPPortInfoList *portinfolist;
    GPPortInfo portinfo;
    
    ret = gp_port_info_list_new(&portinfolist);
    if (!checkError(ret, "create port info list")) {
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    ret = gp_port_info_list_load(portinfolist);
    if (!checkError(ret, "load port info list")) {
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    int index = gp_port_info_list_lookup_path(portinfolist, cameraInfo.port.toUtf8().constData());
    if (index < 0) {
        qWarning() << "Port not found:" << cameraInfo.port;
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    ret = gp_port_info_list_get_info(portinfolist, index, &portinfo);
    if (!checkError(ret, "get port info")) {
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    ret = gp_camera_set_port_info(_camera, portinfo);
    gp_port_info_list_free(portinfolist);
    
    if (!checkError(ret, "set port info")) {
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    // Initialize camera
    ret = gp_camera_init(_camera, _context);
    if (!checkError(ret, "initialize camera")) {
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    emit cameraConnected(cameraInfo);
    return true;
}

void PTPCameraService::disconnect()
{
    if (_camera) {
        if (_isRecording) {
            stopRecording();
        }
        
        gp_camera_exit(_camera, _context);
        gp_camera_free(_camera);
        _camera = nullptr;
        
        emit cameraDisconnected();
    }
}

QImage PTPCameraService::getLiveViewFrame()
{
    if (!_camera) {
        return QImage();
    }
    
    CameraFile *file;
    int ret = gp_file_new(&file);
    if (!checkError(ret, "create file for live view")) {
        return QImage();
    }
    
    ret = gp_camera_capture_preview(_camera, file, _context);
    if (!checkError(ret, "capture preview")) {
        gp_file_free(file);
        return QImage();
    }
    
    const char *data;
    unsigned long size;
    ret = gp_file_get_data_and_size(file, &data, &size);
    if (ret != GP_OK) {
        qWarning() << "Failed to get preview data:" << gp_result_as_string(ret);
        gp_file_free(file);
        return QImage();
    }
    
    // Load JPEG data into QImage
    QImage image;
    image.loadFromData(reinterpret_cast<const uchar*>(data), size, "JPEG");
    
    gp_file_free(file);
    return image;
}

QMap<QString, QVariant> PTPCameraService::getCapabilities()
{
    QMap<QString, QVariant> capabilities;
    
    if (!_camera) {
        return capabilities;
    }
    
    CameraWidget *config;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (!checkError(ret, "get camera config")) {
        return capabilities;
    }
    
    // Get exposure modes and filter to Manual and Aperture Priority
    CameraWidget *exposureMode = findWidget("expprogram");
    if (exposureMode) {
        int count = gp_widget_count_choices(exposureMode);
        QStringList modes;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(exposureMode, i, &choice);
            QString mode = QString::fromUtf8(choice);
            // Only include Manual and Aperture Priority modes
            if (mode.contains("Manual", Qt::CaseInsensitive) || 
                mode.contains("Aperture", Qt::CaseInsensitive) ||
                mode.contains("A", Qt::CaseInsensitive) ||
                mode.contains("M", Qt::CaseInsensitive)) {
                modes.append(mode);
            }
        }
        capabilities["exposureModes"] = modes;
    }
    
    // Get ISO values
    CameraWidget *iso = findWidget("iso");
    if (iso) {
        int count = gp_widget_count_choices(iso);
        QStringList isoValues;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(iso, i, &choice);
            isoValues.append(QString::fromUtf8(choice));
        }
        capabilities["isoValues"] = isoValues;
    }
    
    // Get shutter speeds
    CameraWidget *shutter = findWidget("shutterspeed");
    if (shutter) {
        int count = gp_widget_count_choices(shutter);
        QStringList shutterSpeeds;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(shutter, i, &choice);
            shutterSpeeds.append(QString::fromUtf8(choice));
        }
        capabilities["shutterSpeeds"] = shutterSpeeds;
    }
    
    // Get aperture values
    CameraWidget *aperture = findWidget("aperture");
    if (aperture) {
        int count = gp_widget_count_choices(aperture);
        QStringList apertureValues;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(aperture, i, &choice);
            apertureValues.append(QString::fromUtf8(choice));
        }
        capabilities["apertureValues"] = apertureValues;
    }
    
    gp_widget_free(config);
    return capabilities;
}

bool PTPCameraService::setSetting(const QString& name, const QVariant& value)
{
    if (!_camera) {
        return false;
    }
    
    CameraWidget *config;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (!checkError(ret, "get camera config for setting")) {
        return false;
    }
    
    CameraWidget *widget = findWidget(name);
    if (!widget) {
        qWarning() << "Setting not found:" << name;
        gp_widget_free(config);
        return false;
    }
    
    QString valueStr = value.toString();
    ret = gp_widget_set_value(widget, valueStr.toUtf8().constData());
    
    if (!checkError(ret, "set widget value")) {
        gp_widget_free(config);
        return false;
    }
    
    ret = gp_camera_set_config(_camera, config, _context);
    gp_widget_free(config);
    
    return checkError(ret, "apply camera config");
}

QVariant PTPCameraService::getSetting(const QString& name)
{
    if (!_camera) {
        return QVariant();
    }
    
    CameraWidget *config;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (!checkError(ret, "get camera config for reading")) {
        return QVariant();
    }
    
    CameraWidget *widget = findWidget(name);
    if (!widget) {
        gp_widget_free(config);
        return QVariant();
    }
    
    char *value;
    ret = gp_widget_get_value(widget, &value);
    
    gp_widget_free(config);
    
    if (ret == GP_OK && value) {
        return QString::fromUtf8(value);
    }
    
    return QVariant();
}

QString PTPCameraService::captureImage()
{
    if (!_camera) {
        qWarning() << "PTP camera not connected";
        return QString();
    }
    
    qDebug() << "PTPCameraService::captureImage - Capturing to camera SD card";
    
    // Step 1: Capture image to camera SD card
    CameraFilePath camera_file_path;
    int ret = gp_camera_capture(_camera, GP_CAPTURE_IMAGE, &camera_file_path, _context);
    
    if (!checkError(ret, "capture image to SD card")) {
        return QString();
    }
    
    qDebug() << "Image captured to camera:" << camera_file_path.folder << "/" << camera_file_path.name;
    
    // Step 2: Download image from camera via USB
    CameraFile *file;
    ret = gp_file_new(&file);
    if (!checkError(ret, "create file for USB transfer")) {
        return QString();
    }
    
    ret = gp_camera_file_get(_camera, camera_file_path.folder, camera_file_path.name,
                              GP_FILE_TYPE_NORMAL, file, _context);
    
    if (!checkError(ret, "download file via USB")) {
        gp_file_free(file);
        return QString();
    }
    
    // Step 3: Save to local captures directory with timestamp
    QString capturesPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/uScope/captures";
    QString fileName = QString::fromUtf8(camera_file_path.name);
    
    // Add timestamp prefix to prevent overwrites
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString baseName = QFileInfo(fileName).completeBaseName();
    QString extension = QFileInfo(fileName).suffix();
    QString timestampedFileName = QString("%1_%2.%3").arg(timestamp, baseName, extension);
    
    QString savePath = QString("%1/%2").arg(capturesPath, timestampedFileName);
    
    ret = gp_file_save(file, savePath.toUtf8().constData());
    gp_file_free(file);
    
    if (!checkError(ret, "save file to disk")) {
        return QString();
    }
    
    qDebug() << "Image transferred and saved to:" << savePath;
    emit captureComplete(savePath);
    return savePath;
}

bool PTPCameraService::startRecording()
{
    if (!_camera || _isRecording) {
        return false;
    }
    
    // For PTP cameras, recording is typically done on-camera
    // We trigger movie capture mode
    CameraFilePath camera_file_path;
    int ret = gp_camera_capture(_camera, GP_CAPTURE_MOVIE, &camera_file_path, _context);
    
    if (!checkError(ret, "start recording")) {
        return false;
    }
    
    _isRecording = true;
    return true;
}

bool PTPCameraService::stopRecording()
{
    if (!_camera || !_isRecording) {
        return false;
    }
    
    // Stop movie capture - camera-specific implementation
    _isRecording = false;
    return true;
}

bool PTPCameraService::checkError(int result, const QString& operation)
{
    if (result != GP_OK) {
        QString errorMsg = QString("PTP error during %1: %2")
                               .arg(operation)
                               .arg(gp_result_as_string(result));
        qWarning() << errorMsg;
        emit error(errorMsg);
        return false;
    }
    return true;
}

CameraWidget* PTPCameraService::findWidget(const QString& name)
{
    if (!_camera) {
        return nullptr;
    }
    
    CameraWidget *config;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (ret != GP_OK) {
        return nullptr;
    }
    
    CameraWidget *widget;
    ret = gp_widget_get_child_by_name(config, name.toUtf8().constData(), &widget);
    
    if (ret != GP_OK) {
        // Try by label
        ret = gp_widget_get_child_by_label(config, name.toUtf8().constData(), &widget);
    }
    
    // Note: Don't free config here as widget is part of it
    return (ret == GP_OK) ? widget : nullptr;
}
