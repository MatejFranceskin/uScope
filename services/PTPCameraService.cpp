#include "PTPCameraService.h"
#include <QDebug>
#include <QBuffer>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileInfo>

#ifdef Q_OS_ANDROID
#include "AndroidUsbHelper.h"

// Forward declare libgphoto2 Android USB function
// This is defined in libgphoto2_port when compiled with HAVE_LIBUSB_WRAP_SYS_DEVICE
extern "C" {
    int gp_port_usb_set_sys_device(int fd);
}
#endif

// libgphoto2 error callback
static void gp_context_error_func(GPContext *context, const char *text, void *data)
{
    Q_UNUSED(context);
    Q_UNUSED(data);
    qWarning() << "libgphoto2 error:" << text;
}

// libgphoto2 message callback
static void gp_context_message_func(GPContext *context, const char *text, void *data)
{
    Q_UNUSED(context);
    Q_UNUSED(data);
    qDebug() << "libgphoto2 message:" << text;
}

PTPCameraService::PTPCameraService(QObject *parent)
    : QObject(parent)
    , _context(nullptr)
    , _camera(nullptr)
    , _isRecording(false)
    , _hotplugTimer(nullptr)
    , _liveViewTimer(nullptr)
#ifdef Q_OS_ANDROID
    , _androidUsbFd(-1)
#endif
{
    _context = gp_context_new();
    
    // Set up libgphoto2 callbacks for better debugging
    gp_context_set_error_func(_context, gp_context_error_func, nullptr);
    gp_context_set_message_func(_context, gp_context_message_func, nullptr);
    
    qDebug() << "PTPCameraService::PTPCameraService - initialized libgphoto2 context";
    
    // Create live view timer for continuous frame capture
    _liveViewTimer = new QTimer(this);
    _liveViewTimer->setInterval(33);  // ~30 fps
    QObject::connect(_liveViewTimer, &QTimer::timeout, this, &PTPCameraService::captureLiveViewFrame);
    
    // Create captures directory if it doesn't exist
    QString capturesPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/uScope/captures";
    QDir dir;
    if (!dir.exists(capturesPath)) {
        dir.mkpath(capturesPath);
        qDebug() << "Created captures directory:" << capturesPath;
    }
    
    // Create hot-plug monitoring timer
    _hotplugTimer = new QTimer(this);
    _hotplugTimer->setInterval(2000); // Check every 2 seconds
    QObject::connect(_hotplugTimer, &QTimer::timeout, this, &PTPCameraService::checkCameraConnection);
}

PTPCameraService::~PTPCameraService()
{
    stopHotplugMonitoring();
    disconnect();
    
#ifdef Q_OS_ANDROID
    if (_androidUsbFd >= 0) {
        AndroidUsbHelper::closeDevice(_androidUsbFd);
        _androidUsbFd = -1;
    }
#endif
    
    if (_context) {
        gp_context_unref(_context);
        _context = nullptr;
    }
}

QList<PTPCameraInfo> PTPCameraService::detectCameras()
{
    QList<PTPCameraInfo> cameras;
    
    qDebug() << "PTPCameraService::detectCameras - starting PTP camera detection";
    
#ifdef Q_OS_ANDROID
    // On Android, enumerate USB devices using UsbManager
    QList<AndroidUsbHelper::UsbDeviceInfo> usbDevices = AndroidUsbHelper::getUsbDevices();
    
    qDebug() << "PTPCameraService::detectCameras - found" << usbDevices.size() << "USB devices";
    
    for (const auto& usbDevice : usbDevices) {
        PTPCameraInfo info;
        info.id = QString("android-usb://%1").arg(usbDevice.deviceName);
        info.model = usbDevice.product.isEmpty() ? 
                     QString("USB Camera %1:%2").arg(usbDevice.vendorId, 4, 16, QChar('0'))
                                                  .arg(usbDevice.productId, 4, 16, QChar('0')) :
                     usbDevice.product;
        info.manufacturer = usbDevice.manufacturer;
        info.port = usbDevice.deviceName;  // Store device name in port field
        cameras.append(info);
        
        qDebug() << "Found Android USB camera:" << info.model << "at" << info.port;
    }
    
    return cameras;
#else
    // On desktop platforms, use libgphoto2 autodetect
    qDebug() << "PTPCameraService::detectCameras - using libgphoto2 autodetect";
    
    // Initialize abilities list to ensure camera drivers are loaded
    CameraAbilitiesList *abilities = nullptr;
    int ret = gp_abilities_list_new(&abilities);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - failed to create abilities list:" << gp_result_as_string(ret);
        return cameras;
    }
    
    ret = gp_abilities_list_load(abilities, _context);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - failed to load abilities:" << gp_result_as_string(ret);
        gp_abilities_list_free(abilities);
        return cameras;
    }
    qDebug() << "PTPCameraService::detectCameras - loaded" << gp_abilities_list_count(abilities) << "camera abilities";
    
    // Initialize port info list
    GPPortInfoList *portInfoList = nullptr;
    ret = gp_port_info_list_new(&portInfoList);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - failed to create port info list:" << gp_result_as_string(ret);
        gp_abilities_list_free(abilities);
        return cameras;
    }
    
    ret = gp_port_info_list_load(portInfoList);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - failed to load port info:" << gp_result_as_string(ret);
        gp_port_info_list_free(portInfoList);
        gp_abilities_list_free(abilities);
        return cameras;
    }
    qDebug() << "PTPCameraService::detectCameras - loaded" << gp_port_info_list_count(portInfoList) << "ports";
    
    CameraList *list;
    ret = gp_list_new(&list);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - failed to create camera list:" << gp_result_as_string(ret);
        gp_port_info_list_free(portInfoList);
        gp_abilities_list_free(abilities);
        return cameras;
    }
    
    qDebug() << "PTPCameraService::detectCameras - calling gp_abilities_list_detect with" 
             << gp_abilities_list_count(abilities) << "abilities and" 
             << gp_port_info_list_count(portInfoList) << "ports";
    ret = gp_abilities_list_detect(abilities, portInfoList, list, _context);
    if (ret != GP_OK) {
        qWarning() << "PTPCameraService::detectCameras - abilities_list_detect failed with error code" << ret << ":" << gp_result_as_string(ret);
        gp_list_free(list);
        gp_port_info_list_free(portInfoList);
        gp_abilities_list_free(abilities);
        return cameras;
    }
    
    int count = gp_list_count(list);
    qDebug() << "PTPCameraService::detectCameras - gp_abilities_list_detect found" << count << "PTP cameras";
    
    for (int i = 0; i < count; i++) {
        const char *name = nullptr;
        const char *port = nullptr;
        
        gp_list_get_name(list, i, &name);
        gp_list_get_value(list, i, &port);
        
        qDebug() << "PTPCameraService::detectCameras - camera" << i << ":" 
                 << "name =" << (name ? name : "null")
                 << "port =" << (port ? port : "null");
        
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
            
            qDebug() << "PTPCameraService::detectCameras - added PTP camera:" << info.model 
                     << "manufacturer:" << info.manufacturer << "port:" << info.port;
            
            cameras.append(info);
        }
    }
    
    gp_list_free(list);
    gp_port_info_list_free(portInfoList);
    gp_abilities_list_free(abilities);
    
    qDebug() << "PTPCameraService::detectCameras - returning" << cameras.size() << "PTP cameras";
    return cameras;
#endif // Q_OS_ANDROID
}

bool PTPCameraService::connect(const PTPCameraInfo& cameraInfo)
{
    qDebug() << "PTPCameraService::connect - attempting to connect to" << cameraInfo.model << "at" << cameraInfo.port;
    
    if (_camera) {
        qDebug() << "PTPCameraService::connect - disconnecting existing camera first";
        disconnect();
    }
    
    int ret = gp_camera_new(&_camera);
    if (!checkError(ret, "create camera")) {
        qDebug() << "PTPCameraService::connect - failed to create camera object";
        return false;
    }
    
    qDebug() << "PTPCameraService::connect - camera object created successfully";
    
#ifdef Q_OS_ANDROID
    // On Android, use USB file descriptor approach
    _androidDeviceName = cameraInfo.port;  // port contains device name
    
    // Request permission if needed
    if (!AndroidUsbHelper::hasPermission(_androidDeviceName)) {
        qDebug() << "Requesting USB permission for" << _androidDeviceName;
        AndroidUsbHelper::requestPermission(_androidDeviceName);
        // Permission request is async, connection will fail for now
        // User should retry after granting permission
        emit error("USB permission required. Please grant permission and try again.");
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    // Open USB device and get file descriptor
    _androidUsbFd = AndroidUsbHelper::openDevice(_androidDeviceName);
    if (_androidUsbFd < 0) {
        qWarning() << "Failed to open USB device:" << _androidDeviceName;
        emit error("Failed to open USB device");
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    // Set the USB device file descriptor globally before initializing
    // gp_port_usb_set_sys_device is a global function in libgphoto2
    ret = gp_port_usb_set_sys_device(_androidUsbFd);
    if (!checkError(ret, "set USB device file descriptor")) {
        AndroidUsbHelper::closeDevice(_androidUsbFd);
        _androidUsbFd = -1;
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    qDebug() << "Set USB FD" << _androidUsbFd << "for camera";
#else
    // On desktop platforms, use standard port detection
    qDebug() << "PTPCameraService::connect - desktop platform, setting up port info";
    
    GPPortInfoList *portinfolist;
    GPPortInfo portinfo;
    
    ret = gp_port_info_list_new(&portinfolist);
    if (!checkError(ret, "create port info list")) {
        qDebug() << "PTPCameraService::connect - failed to create port info list";
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    ret = gp_port_info_list_load(portinfolist);
    if (!checkError(ret, "load port info list")) {
        qDebug() << "PTPCameraService::connect - failed to load port info list";
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    qDebug() << "PTPCameraService::connect - looking up port:" << cameraInfo.port;
    
    int index = gp_port_info_list_lookup_path(portinfolist, cameraInfo.port.toUtf8().constData());
    if (index < 0) {
        qWarning() << "PTPCameraService::connect - port not found:" << cameraInfo.port;
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    qDebug() << "PTPCameraService::connect - port found at index:" << index;
    
    ret = gp_port_info_list_get_info(portinfolist, index, &portinfo);
    if (!checkError(ret, "get port info")) {
        qDebug() << "PTPCameraService::connect - failed to get port info";
        gp_port_info_list_free(portinfolist);
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    ret = gp_camera_set_port_info(_camera, portinfo);
    gp_port_info_list_free(portinfolist);
    
    if (!checkError(ret, "set port info")) {
        qDebug() << "PTPCameraService::connect - failed to set port info";
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    qDebug() << "PTPCameraService::connect - port info set successfully";
#endif // Q_OS_ANDROID
    
    // Initialize camera
    ret = gp_camera_init(_camera, _context);
    if (!checkError(ret, "initialize camera")) {
#ifdef Q_OS_ANDROID
        AndroidUsbHelper::closeDevice(_androidUsbFd);
        _androidUsbFd = -1;
#endif
        gp_camera_free(_camera);
        _camera = nullptr;
        return false;
    }
    
    qDebug() << "PTPCameraService::connect - camera initialized, starting live view";
    
    // Start live view timer
    _liveViewTimer->start();
    
    emit cameraConnected(cameraInfo);
    return true;
}

void PTPCameraService::disconnect()
{
    if (_camera) {
        // Stop live view timer
        _liveViewTimer->stop();
        
        if (_isRecording) {
            stopRecording();
        }
        
        qDebug() << "PTPCameraService::disconnect - disconnecting camera";
        
        gp_camera_exit(_camera, _context);
        gp_camera_free(_camera);
        _camera = nullptr;
        
#ifdef Q_OS_ANDROID
        if (_androidUsbFd >= 0) {
            AndroidUsbHelper::closeDevice(_androidUsbFd);
            _androidUsbFd = -1;
        }
        _androidDeviceName.clear();
#endif
        
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
    
    qDebug() << "PTPCameraService::getCapabilities - called";
    
    if (!_camera) {
        qDebug() << "PTPCameraService::getCapabilities - no camera connected";
        return capabilities;
    }
    
    CameraWidget *config;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (!checkError(ret, "get camera config")) {
        qDebug() << "PTPCameraService::getCapabilities - failed to get config";
        return capabilities;
    }
    
    qDebug() << "PTPCameraService::getCapabilities - got config successfully";
    
    // Get exposure modes and filter to Manual and Aperture Priority
    // Try standard widget names first
    CameraWidget *exposureMode = findWidget("expprogram");
    if (!exposureMode) {
        exposureMode = findWidget("exposuremode");
    }
    if (!exposureMode) {
        exposureMode = findWidget("autoexposuremode");
    }
    
    // If standard names not found, search through all widgets for one that looks like exposure mode
    if (!exposureMode) {
        qDebug() << "PTPCameraService::getCapabilities - searching all widgets for exposure mode";
        
        // Enumerate all child widgets of config
        int childCount = gp_widget_count_children(config);
        for (int i = 0; i < childCount && !exposureMode; i++) {
            CameraWidget *section;
            gp_widget_get_child(config, i, &section);
            
            int sectionChildCount = gp_widget_count_children(section);
            for (int j = 0; j < sectionChildCount && !exposureMode; j++) {
                CameraWidget *widget;
                gp_widget_get_child(section, j, &widget);
                
                const char *label;
                gp_widget_get_label(widget, &label);
                QString labelStr = QString::fromUtf8(label);
                
                // Check if label contains "exposure" and "mode" or "program"
                if ((labelStr.contains("Exposure", Qt::CaseInsensitive) && 
                     labelStr.contains("Mode", Qt::CaseInsensitive)) ||
                    labelStr.contains("ExposureMode", Qt::CaseInsensitive) ||
                    (labelStr.contains("Exposure", Qt::CaseInsensitive) && 
                     labelStr.contains("Program", Qt::CaseInsensitive))) {
                    
                    qDebug() << "PTPCameraService::getCapabilities - found potential exposure mode widget:" << labelStr;
                    exposureMode = widget;
                    break;
                }
            }
        }
    }
    if (exposureMode) {
        // Store the widget name for later use when setting values
        const char *widgetName;
        gp_widget_get_name(exposureMode, &widgetName);
        _exposureModeWidgetName = QString::fromUtf8(widgetName);
        qDebug() << "PTPCameraService::getCapabilities - exposure mode widget name:" << _exposureModeWidgetName;
        
        int count = gp_widget_count_choices(exposureMode);
        QStringList modes;
        QStringList allModes;  // For debugging
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(exposureMode, i, &choice);
            QString mode = QString::fromUtf8(choice);
            allModes.append(mode);  // Collect all for debug
            
            // Map Sony numeric codes to readable names and filter for microscopy
            QString readableMode;
            bool isSonyNumeric = false;
            int modeValue = mode.toInt(&isSonyNumeric);
            
            if (isSonyNumeric) {
                // Sony PTP numeric exposure modes
                switch (modeValue) {
                    case 1: readableMode = "Manual"; break;
                    case 2: readableMode = "Program Auto"; break;
                    case 3: readableMode = "Aperture Priority"; break;
                    case 4: readableMode = "Shutter Priority"; break;
                    case 32768:
                    case 32769: readableMode = "Auto"; break;
                    default: readableMode = mode; break;
                }
            } else {
                readableMode = mode;
            }
            
            // Only include Manual and Aperture Priority modes for microscopy
            if (readableMode.contains("Manual", Qt::CaseInsensitive) || 
                readableMode.contains("Aperture", Qt::CaseInsensitive) ||
                readableMode == "M" || readableMode == "A" ||
                modeValue == 1 || modeValue == 3) {  // Manual=1, Aperture Priority=3
                modes.append(readableMode);
            }
        }
        qDebug() << "PTPCameraService::getCapabilities - all exposure modes:" << allModes;
        qDebug() << "PTPCameraService::getCapabilities - filtered exposure modes:" << modes;
        capabilities["exposuremode"] = modes;  // Match UI expectation
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
        capabilities["iso"] = isoValues;  // Match UI expectation
    }
    
    // Get shutter speeds
    CameraWidget *shutter = findWidget("shutterspeed");
    if (!shutter) {
        shutter = findWidget("shutterspeed2");
    }
    if (!shutter) {
        shutter = findWidget("eos-shutterspeed");
    }
    
    // If standard names not found, search all widgets
    if (!shutter) {
        int childCount = gp_widget_count_children(config);
        for (int i = 0; i < childCount && !shutter; i++) {
            CameraWidget *section;
            gp_widget_get_child(config, i, &section);
            
            int sectionChildCount = gp_widget_count_children(section);
            for (int j = 0; j < sectionChildCount && !shutter; j++) {
                CameraWidget *widget;
                gp_widget_get_child(section, j, &widget);
                
                const char *label;
                gp_widget_get_label(widget, &label);
                QString labelStr = QString::fromUtf8(label);
                
                if (labelStr.contains("Shutter", Qt::CaseInsensitive) && 
                    (labelStr.contains("Speed", Qt::CaseInsensitive) || labelStr == "Shutter speed")) {
                    qDebug() << "PTPCameraService::getCapabilities - found shutter speed widget:" << labelStr;
                    shutter = widget;
                    break;
                }
            }
        }
    }
    
    if (shutter) {
        const char *widgetName;
        gp_widget_get_name(shutter, &widgetName);
        _shutterSpeedWidgetName = QString::fromUtf8(widgetName);
        int count = gp_widget_count_choices(shutter);
        QStringList shutterSpeeds;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(shutter, i, &choice);
            QString rawValue = QString::fromUtf8(choice);
            QString readable = convertShutterSpeedToReadable(rawValue);
            shutterSpeeds.append(readable);
        }
        qDebug() << "PTPCameraService::getCapabilities - shutter speed values:" << shutterSpeeds;
        capabilities["shutterspeed"] = shutterSpeeds;  // Match UI expectation
    }
    
    // Get exposure compensation values
    CameraWidget *exposureComp = findWidget("exposurecompensation");
    if (exposureComp) {
        int count = gp_widget_count_choices(exposureComp);
        QStringList expCompValues;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(exposureComp, i, &choice);
            expCompValues.append(QString::fromUtf8(choice));
        }
        capabilities["exposurecompensation"] = expCompValues;  // Match UI expectation
    }
    
    // Get white balance modes
    CameraWidget *whiteBalance = findWidget("whitebalance");
    if (!whiteBalance) {
        whiteBalance = findWidget("wb");
    }
    if (!whiteBalance) {
        whiteBalance = findWidget("eos-whitebalance");
    }
    
    // If standard names not found, search all widgets
    if (!whiteBalance) {
        int childCount = gp_widget_count_children(config);
        for (int i = 0; i < childCount && !whiteBalance; i++) {
            CameraWidget *section;
            gp_widget_get_child(config, i, &section);
            
            int sectionChildCount = gp_widget_count_children(section);
            for (int j = 0; j < sectionChildCount && !whiteBalance; j++) {
                CameraWidget *widget;
                gp_widget_get_child(section, j, &widget);
                
                const char *label;
                gp_widget_get_label(widget, &label);
                QString labelStr = QString::fromUtf8(label);
                
                if (labelStr.contains("WhiteBalance", Qt::CaseInsensitive) ||
                    (labelStr.contains("White", Qt::CaseInsensitive) && labelStr.contains("Balance", Qt::CaseInsensitive)) ||
                    labelStr == "WB") {
                    qDebug() << "PTPCameraService::getCapabilities - found white balance widget:" << labelStr;
                    whiteBalance = widget;
                    break;
                }
            }
        }
    }
    
    if (whiteBalance) {
        const char *widgetName;
        gp_widget_get_name(whiteBalance, &widgetName);
        _whiteBalanceWidgetName = QString::fromUtf8(widgetName);
        int count = gp_widget_count_choices(whiteBalance);
        QStringList wbModes;
        for (int i = 0; i < count; i++) {
            const char *choice;
            gp_widget_get_choice(whiteBalance, i, &choice);
            QString rawValue = QString::fromUtf8(choice);
            QString readable = convertWhiteBalanceToReadable(rawValue);
            wbModes.append(readable);
        }
        qDebug() << "PTPCameraService::getCapabilities - white balance values:" << wbModes;
        capabilities["whitebalance"] = wbModes;  // Match UI expectation
    }
    
    gp_widget_free(config);
    return capabilities;
}

bool PTPCameraService::setSetting(const QString& name, const QVariant& value)
{
    if (!_camera) {
        return false;
    }
    
    // Use cached widget name if available
    QString widgetName = name;
    if (name == "exposuremode" && !_exposureModeWidgetName.isEmpty()) {
        widgetName = _exposureModeWidgetName;
    } else if (name == "shutterspeed" && !_shutterSpeedWidgetName.isEmpty()) {
        widgetName = _shutterSpeedWidgetName;
    } else if (name == "whitebalance" && !_whiteBalanceWidgetName.isEmpty()) {
        widgetName = _whiteBalanceWidgetName;
    }
    
    QString valueStr = value.toString();
    QString originalValue = valueStr;
    
    // Special handling for exposure mode - convert readable names to numeric codes
    if (name == "exposuremode") {
        if (valueStr == "Manual") valueStr = "1";
        else if (valueStr == "Program Auto") valueStr = "2";
        else if (valueStr == "Aperture Priority") valueStr = "3";
        else if (valueStr == "Shutter Priority") valueStr = "4";
        else if (valueStr == "Auto") valueStr = "32768";
    }
    // Convert shutter speed from readable format ("1/250") to numeric code
    else if (name == "shutterspeed") {
        valueStr = convertReadableToShutterSpeed(valueStr);
    }
    // Convert white balance from readable format ("Daylight") to numeric code
    else if (name == "whitebalance") {
        valueStr = convertReadableToWhiteBalance(valueStr);
    }
    
    qDebug() << "PTPCameraService::setSetting -" << name << "readable:" << originalValue << "raw:" << valueStr;
    
    // Try gp_camera_get_single_config first (recommended by gphoto2)
    CameraWidget *widget = nullptr;
    int ret = gp_camera_get_single_config(_camera, widgetName.toUtf8().constData(), &widget, _context);
    
    if (ret == GP_OK && widget) {
        // Set the value
        ret = gp_widget_set_value(widget, valueStr.toUtf8().constData());
        
        if (ret == GP_OK) {
            // Apply using gp_camera_set_single_config
            ret = gp_camera_set_single_config(_camera, widgetName.toUtf8().constData(), widget, _context);
            if (!checkError(ret, "apply single config")) {
                gp_widget_free(widget);
                return false;
            }
        } else {
            checkError(ret, "set widget value");
            gp_widget_free(widget);
            return false;
        }
        
        gp_widget_free(widget);
    } else {
        // Fallback: use full config tree
        qDebug() << "PTPCameraService::setSetting - single config failed, using full tree";
        
        CameraWidget *config;
        ret = gp_camera_get_config(_camera, &config, _context);
        if (!checkError(ret, "get camera config for setting")) {
            return false;
        }
        
        widget = findWidget(widgetName);
        if (!widget) {
            qWarning() << "Setting not found:" << widgetName << "(requested:" << name << ")";
            gp_widget_free(config);
            return false;
        }
        
        ret = gp_widget_set_value(widget, valueStr.toUtf8().constData());
        if (!checkError(ret, "set widget value")) {
            gp_widget_free(config);
            return false;
        }
        
        ret = gp_camera_set_config(_camera, config, _context);
        gp_widget_free(config);
        
        if (!checkError(ret, "apply camera config")) {
            return false;
        }
    }
    
    // Pause and restart live view to apply settings to preview
    _liveViewTimer->stop();
    QTimer::singleShot(100, this, [this]() {
        if (_camera) {
            _liveViewTimer->start();
        }
    });
    
    return true;
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
    
    // Use cached widget name if available
    QString widgetName = name;
    if (name == "exposuremode" && !_exposureModeWidgetName.isEmpty()) {
        widgetName = _exposureModeWidgetName;
    } else if (name == "shutterspeed" && !_shutterSpeedWidgetName.isEmpty()) {
        widgetName = _shutterSpeedWidgetName;
    } else if (name == "whitebalance" && !_whiteBalanceWidgetName.isEmpty()) {
        widgetName = _whiteBalanceWidgetName;
    }
    
    CameraWidget *widget = findWidget(widgetName);
    if (!widget) {
        gp_widget_free(config);
        return QVariant();
    }
    
    char *value;
    ret = gp_widget_get_value(widget, &value);
    
    gp_widget_free(config);
    
    if (ret == GP_OK && value) {
        QString rawValue = QString::fromUtf8(value);
        QString readableValue = rawValue;
        
        // Convert numeric values to readable format
        if (name == "shutterspeed") {
            readableValue = convertShutterSpeedToReadable(rawValue);
        } else if (name == "whitebalance") {
            readableValue = convertWhiteBalanceToReadable(rawValue);
        } else if (name == "exposuremode") {
            // Convert numeric exposure mode to readable
            bool isNumeric;
            int modeValue = rawValue.toInt(&isNumeric);
            if (isNumeric) {
                switch (modeValue) {
                    case 1: readableValue = "Manual"; break;
                    case 2: readableValue = "Program Auto"; break;
                    case 3: readableValue = "Aperture Priority"; break;
                    case 4: readableValue = "Shutter Priority"; break;
                    case 32768:
                    case 32769: readableValue = "Auto"; break;
                }
            }
        }
        
        qDebug() << "PTPCameraService::getSetting -" << name << "raw:" << rawValue << "readable:" << readableValue;
        return readableValue;
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
    
    // Embed EXIF metadata with camera settings
    embedExifMetadata(savePath);
    
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
    if (result == GP_OK) {
        return true;
    }

    QString errorMsg;
    bool isCritical = true;

    // Handle specific error codes with user-friendly messages
    switch (result) {
    case GP_ERROR_CAMERA_BUSY:
        errorMsg = QString("Camera is busy during %1. Please wait and try again.").arg(operation);
        isCritical = false;
        break;
    
    case GP_ERROR_NO_SPACE:
        errorMsg = QString("Storage full on camera SD card. Please free up space or use a different card.");
        break;
    
    case GP_ERROR_FILE_NOT_FOUND:
        errorMsg = QString("File or folder not found on camera during %1.").arg(operation);
        break;
    
    case GP_ERROR_IO:
    case GP_ERROR_IO_USB_FIND:
    case GP_ERROR_IO_USB_CLAIM:
        errorMsg = QString("USB communication error during %1. Check cable connection.").arg(operation);
        break;
    
    case GP_ERROR_TIMEOUT:
        errorMsg = QString("Camera timeout during %1. The camera may be unresponsive.").arg(operation);
        break;
    
    case GP_ERROR_NOT_SUPPORTED:
        errorMsg = QString("Operation '%1' is not supported by this camera model.").arg(operation);
        break;
    
    case GP_ERROR_CANCEL:
        errorMsg = QString("Operation cancelled by user.");
        isCritical = false;
        break;
    
    default:
        errorMsg = QString("PTP error during %1: %2 (code %3)")
                       .arg(operation)
                       .arg(gp_result_as_string(result))
                       .arg(result);
        break;
    }

    qWarning() << errorMsg;
    
    if (isCritical) {
        emit error(errorMsg);
    } else {
        emit warning(errorMsg);
    }
    
    return false;
}

// Helper function to convert Sony's numeric shutter speed to readable format
QString PTPCameraService::convertShutterSpeedToReadable(const QString& rawValue)
{
    bool ok;
    int value = rawValue.toInt(&ok);
    
    // If it's not a number, it's already readable (Canon/Nikon/Fuji return text like "1/250", "0.0040s")
    if (!ok) return rawValue;
    
    // Sony encodes shutter speed as: high 16 bits = numerator, low 16 bits = denominator
    // Special values: -2 = bulb, very high values = special modes
    if (value == -2) return "Bulb";
    if (value >= 10000000) return rawValue; // Unknown format, return as-is
    
    int numerator = (value >> 16) & 0xFFFF;
    int denominator = value & 0xFFFF;
    
    if (numerator == 0 && denominator > 0) {
        // Fractional second (1/denominator)
        return QString("1/%1").arg(denominator);
    } else if (numerator > 0 && denominator == 0) {
        // Whole seconds
        return QString("%1s").arg(numerator);
    } else if (numerator > 0 && denominator > 0) {
        // General fraction
        return QString("%1/%2").arg(numerator).arg(denominator);
    }
    
    return rawValue; // Fallback
}

// Helper function to convert readable shutter speed back to Sony's numeric format
QString PTPCameraService::convertReadableToShutterSpeed(const QString& readable)
{
    // Check if it's already numeric (Canon/Nikon/Fuji cameras use text values directly)
    bool ok;
    readable.toInt(&ok);
    if (ok) return readable; // Already numeric, pass through
    
    // Convert text to Sony's numeric format
    if (readable == "Bulb") return "-2";
    
    // Try to parse formats like "1/250", "1/15", "2s", etc.
    if (readable.contains('/')) {
        QStringList parts = readable.split('/');
        if (parts.size() == 2) {
            int num = parts[0].toInt();
            int denom = parts[1].toInt();
            return QString::number((num << 16) | denom);
        }
    } else if (readable.endsWith('s')) {
        int seconds = readable.left(readable.length() - 1).toInt();
        return QString::number(seconds << 16);
    }
    
    return readable; // Return as-is (Canon/Nikon cameras expect text like "1/250")
}

// Helper function to convert Sony's numeric white balance to readable format
QString PTPCameraService::convertWhiteBalanceToReadable(const QString& rawValue)
{
    bool ok;
    int value = rawValue.toInt(&ok);
    
    // If it's not a number, it's already readable (Canon/Nikon/Fuji return text like "Auto", "Daylight")
    if (!ok) return rawValue;
    
    // Common white balance codes (may vary by manufacturer)
    switch (value) {
        case 2: return "Auto";
        case 4: return "Daylight";
        case 6: return "Cloudy";
        case 7: return "Shade";
        case 8: return "Tungsten";
        case 32768: return "Flash";
        case 32769: return "Fluorescent";
        case 32770: return "Tungsten";
        case 32771: return "Fluorescent";
        case 32772: return "Flash";
        case 32784: return "Shade";
        case 32785: return "Color Temperature";
        case 32786: return "Custom";
        default: return rawValue; // Return numeric value if unknown
    }
}

// Helper function to convert readable white balance back to Sony's numeric format
QString PTPCameraService::convertReadableToWhiteBalance(const QString& readable)
{
    // Check if it's already numeric  
    bool ok;
    readable.toInt(&ok);
    if (ok) return readable; // Already numeric, pass through
    
    // Convert text to Sony's numeric format (only for Sony cameras that need it)
    if (readable == "Auto") return "2";
    if (readable == "Daylight") return "4";
    if (readable == "Cloudy") return "6";
    if (readable == "Shade") return "32784";
    if (readable == "Tungsten") return "32770";
    if (readable == "Fluorescent") return "32771";
    if (readable == "Flash") return "32772";
    if (readable == "Custom") return "32786";
    
    return readable; // Return as-is (Canon/Nikon/Fuji cameras expect text like "Auto")
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

void PTPCameraService::startHotplugMonitoring()
{
    if (_hotplugTimer && !_hotplugTimer->isActive()) {
        qDebug() << "Starting PTP camera hot-plug monitoring (2 second interval)";
        _lastDetectedCameras = detectCameras();
        _hotplugTimer->start();
    }
}

void PTPCameraService::stopHotplugMonitoring()
{
    if (_hotplugTimer && _hotplugTimer->isActive()) {
        qDebug() << "Stopping PTP camera hot-plug monitoring";
        _hotplugTimer->stop();
    }
}

void PTPCameraService::checkCameraConnection()
{
    QList<PTPCameraInfo> currentCameras = detectCameras();
    
    // Check for newly plugged cameras
    for (const PTPCameraInfo& camera : currentCameras) {
        bool found = false;
        for (const PTPCameraInfo& lastCamera : _lastDetectedCameras) {
            if (camera.id == lastCamera.id) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            qDebug() << "PTP camera plugged:" << camera.manufacturer << camera.model;
            emit cameraPlugged(camera);
        }
    }
    
    // Check for unplugged cameras
    for (const PTPCameraInfo& lastCamera : _lastDetectedCameras) {
        bool found = false;
        for (const PTPCameraInfo& camera : currentCameras) {
            if (camera.id == lastCamera.id) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            qDebug() << "PTP camera unplugged:" << lastCamera.manufacturer << lastCamera.model;
            emit cameraUnplugged(lastCamera.id);
            
            // If the unplugged camera was the connected one, disconnect
            if (_camera && lastCamera.port == getCurrentCameraPort()) {
                qWarning() << "Active camera was unplugged, disconnecting";
                disconnect();
            }
        }
    }
    
    _lastDetectedCameras = currentCameras;
    
    // Check battery level if camera is connected
    if (_camera) {
        int batteryLevel = getBatteryLevel();
        if (batteryLevel >= 0 && batteryLevel <= 20) {
            QString msg = QString("Camera battery low: %1%").arg(batteryLevel);
            qWarning() << msg;
            emit warning(msg);
        }
    }
}

int PTPCameraService::getBatteryLevel()
{
    if (!_camera) {
        return -1;
    }
    
    CameraWidget *config = nullptr;
    int ret = gp_camera_get_config(_camera, &config, _context);
    if (ret != GP_OK) {
        return -1;
    }
    
    CameraWidget *batteryWidget = nullptr;
    ret = gp_widget_get_child_by_name(config, "batterylevel", &batteryWidget);
    if (ret != GP_OK) {
        // Try alternative battery widget names
        ret = gp_widget_get_child_by_name(config, "battery", &batteryWidget);
        if (ret != GP_OK) {
            ret = gp_widget_get_child_by_name(config, "batterypower", &batteryWidget);
        }
    }
    
    int batteryLevel = -1;
    if (ret == GP_OK && batteryWidget) {
        CameraWidgetType type;
        gp_widget_get_type(batteryWidget, &type);
        
        if (type == GP_WIDGET_TEXT || type == GP_WIDGET_MENU) {
            char *value = nullptr;
            ret = gp_widget_get_value(batteryWidget, &value);
            if (ret == GP_OK && value) {
                QString batteryStr = QString::fromUtf8(value);
                // Try to parse percentage (e.g., "80%", "80", "High", etc.)
                if (batteryStr.contains("%")) {
                    batteryStr.remove("%");
                }
                bool ok;
                int parsed = batteryStr.toInt(&ok);
                if (ok && parsed >= 0 && parsed <= 100) {
                    batteryLevel = parsed;
                } else {
                    // Handle text values like "High", "Medium", "Low"
                    QString lower = batteryStr.toLower();
                    if (lower.contains("high") || lower.contains("full")) {
                        batteryLevel = 100;
                    } else if (lower.contains("medium") || lower.contains("normal")) {
                        batteryLevel = 50;
                    } else if (lower.contains("low")) {
                        batteryLevel = 20;
                    }
                }
            }
        } else if (type == GP_WIDGET_RANGE) {
            float value;
            ret = gp_widget_get_value(batteryWidget, &value);
            if (ret == GP_OK) {
                batteryLevel = static_cast<int>(value);
            }
        }
    }
    
    gp_widget_free(config);
    return batteryLevel;
}

QString PTPCameraService::getCurrentCameraPort() const
{
    if (!_camera) {
        return QString();
    }
    
    GPPortInfo portinfo;
    int ret = gp_camera_get_port_info(_camera, &portinfo);
    if (ret != GP_OK) {
        return QString();
    }
    
    char *path;
    ret = gp_port_info_get_path(portinfo, &path);
    if (ret == GP_OK && path) {
        return QString::fromUtf8(path);
    }
    
    return QString();
}

QString PTPCameraService::getCameraManufacturer()
{
    if (!_camera) {
        return QString();
    }
    
    CameraAbilities abilities;
    int ret = gp_camera_get_abilities(_camera, &abilities);
    if (ret == GP_OK) {
        // abilities.model contains "Manufacturer Model" format
        QString model = QString::fromUtf8(abilities.model);
        // Extract manufacturer (first word typically)
        QStringList parts = model.split(' ', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            return parts.first();
        }
    }
    
    return QString();
}

QString PTPCameraService::getCameraModel()
{
    if (!_camera) {
        return QString();
    }
    
    CameraAbilities abilities;
    int ret = gp_camera_get_abilities(_camera, &abilities);
    if (ret == GP_OK) {
        return QString::fromUtf8(abilities.model);
    }
    
    return QString();
}

void PTPCameraService::embedExifMetadata(const QString& filePath)
{
    if (!_camera) {
        return;
    }
    
    // Get camera manufacturer and model
    QString manufacturer = getCameraManufacturer();
    QString model = getCameraModel();
    
    // Get current camera settings
    QString iso = getSetting("iso").toString();
    QString shutterSpeed = getSetting("shutterspeed").toString();
    QString aperture = getSetting("aperture").toString();
    QString whiteBalance = getSetting("whitebalance").toString();
    
    qDebug() << "Camera settings for captured image:";
    qDebug() << "  Camera:" << manufacturer << model;
    qDebug() << "  ISO:" << iso;
    qDebug() << "  Shutter:" << shutterSpeed;
    qDebug() << "  Aperture:" << aperture;
    qDebug() << "  White Balance:" << whiteBalance;
    
    // Note: Most DSLR cameras already embed comprehensive EXIF data
    // including ISO, shutter speed, aperture, focal length, white balance,
    // timestamp, and camera model when capturing images.
    // The EXIF data is written by the camera itself to the image file
    // on the SD card before we transfer it via USB.
    // Therefore, the transferred file already contains all necessary metadata.
    
    // If we needed to modify or add custom EXIF data, we would use a library
    // like exiv2 (C++ native) or libexif with proper JPEG parsing to insert
    // the EXIF segment into the JPEG file structure.
}

void PTPCameraService::captureLiveViewFrame()
{
    if (!_camera) {
        return;
    }
    
    QImage frame = getLiveViewFrame();
    if (!frame.isNull()) {
        emit frameReady(frame);
    }
}
