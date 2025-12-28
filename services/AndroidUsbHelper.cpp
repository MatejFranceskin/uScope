#include "AndroidUsbHelper.h"
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QJniEnvironment>
#include <QCoreApplication>
#include <unistd.h>

AndroidUsbHelper::AndroidUsbHelper(QObject *parent)
    : QObject(parent)
{
}

AndroidUsbHelper::~AndroidUsbHelper()
{
}

QJniObject AndroidUsbHelper::getUsbManager()
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    
    QJniObject usbService = QJniObject::getStaticObjectField(
        "android/content/Context",
        "USB_SERVICE",
        "Ljava/lang/String;"
    );
    
    QJniObject usbManager = activity.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        usbService.object<jstring>()
    );
    
    return usbManager;
}

QList<AndroidUsbHelper::UsbDeviceInfo> AndroidUsbHelper::getUsbDevices()
{
    QList<UsbDeviceInfo> devices;
    
    QJniObject usbManager = getUsbManager();
    if (!usbManager.isValid()) {
        qWarning() << "Failed to get USB manager";
        return devices;
    }
    
    // Get device list HashMap<String, UsbDevice>
    QJniObject deviceList = usbManager.callObjectMethod(
        "getDeviceList",
        "()Ljava/util/HashMap;"
    );
    
    if (!deviceList.isValid()) {
        qWarning() << "Failed to get device list";
        return devices;
    }
    
    // Get HashMap values (Collection<UsbDevice>)
    QJniObject values = deviceList.callObjectMethod(
        "values",
        "()Ljava/util/Collection;"
    );
    
    // Convert to array
    QJniObject valuesArray = values.callObjectMethod(
        "toArray",
        "()[Ljava/lang/Object;"
    );
    
    QJniEnvironment env;
    jobjectArray jArray = valuesArray.object<jobjectArray>();
    jsize length = env->GetArrayLength(jArray);
    
    for (jsize i = 0; i < length; ++i) {
        QJniObject device = env->GetObjectArrayElement(jArray, i);
        
        UsbDeviceInfo info;
        info.vendorId = device.callMethod<jint>("getVendorId");
        info.productId = device.callMethod<jint>("getProductId");
        
        QJniObject deviceName = device.callObjectMethod(
            "getDeviceName",
            "()Ljava/lang/String;"
        );
        info.deviceName = deviceName.toString();
        
        // Get manufacturer (may be null)
        QJniObject manufacturer = device.callObjectMethod(
            "getManufacturerName",
            "()Ljava/lang/String;"
        );
        if (manufacturer.isValid()) {
            info.manufacturer = manufacturer.toString();
        }
        
        // Get product name (may be null)
        QJniObject product = device.callObjectMethod(
            "getProductName",
            "()Ljava/lang/String;"
        );
        if (product.isValid()) {
            info.product = product.toString();
        }
        
        // Only add devices that look like cameras (imaging class)
        jint deviceClass = device.callMethod<jint>("getDeviceClass");
        jint deviceSubclass = device.callMethod<jint>("getDeviceSubclass");
        
        // USB_CLASS_STILL_IMAGE = 6, or USB_CLASS_PTP = 6
        // Also check for vendor-specific (255) as some cameras use that
        if (deviceClass == 6 || deviceClass == 255) {
            devices.append(info);
            qDebug() << "Found USB imaging device:" << info.deviceName 
                     << "VID:" << QString::number(info.vendorId, 16)
                     << "PID:" << QString::number(info.productId, 16);
        }
        
        env->DeleteLocalRef(device.object());
    }
    
    return devices;
}

bool AndroidUsbHelper::hasPermission(const QString& deviceName)
{
    QJniObject usbManager = getUsbManager();
    if (!usbManager.isValid()) {
        return false;
    }
    
    // Get device by name
    QJniObject jDeviceName = QJniObject::fromString(deviceName);
    QJniObject deviceList = usbManager.callObjectMethod(
        "getDeviceList",
        "()Ljava/util/HashMap;"
    );
    
    QJniObject device = deviceList.callObjectMethod(
        "get",
        "(Ljava/lang/Object;)Ljava/lang/Object;",
        jDeviceName.object<jobject>()
    );
    
    if (!device.isValid()) {
        qWarning() << "Device not found:" << deviceName;
        return false;
    }
    
    // Check permission
    jboolean hasPermission = usbManager.callMethod<jboolean>(
        "hasPermission",
        "(Landroid/hardware/usb/UsbDevice;)Z",
        device.object<jobject>()
    );
    
    return hasPermission;
}

bool AndroidUsbHelper::requestPermission(const QString& deviceName)
{
    // Check if we already have permission
    if (hasPermission(deviceName)) {
        qDebug() << "Already have permission for" << deviceName;
        return true;
    }
    
    QJniObject usbManager = getUsbManager();
    if (!usbManager.isValid()) {
        return false;
    }
    
    // Get device by name
    QJniObject jDeviceName = QJniObject::fromString(deviceName);
    QJniObject deviceList = usbManager.callObjectMethod(
        "getDeviceList",
        "()Ljava/util/HashMap;"
    );
    
    QJniObject device = deviceList.callObjectMethod(
        "get",
        "(Ljava/lang/Object;)Ljava/lang/Object;",
        jDeviceName.object<jobject>()
    );
    
    if (!device.isValid()) {
        qWarning() << "Device not found:" << deviceName;
        return false;
    }
    
    // Create PendingIntent for permission broadcast
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QString action = "org.uscope.USB_PERMISSION";
    QJniObject jAction = QJniObject::fromString(action);
    
    QJniObject intent = QJniObject("android/content/Intent",
                                     "(Ljava/lang/String;)V",
                                     jAction.object<jstring>());
    
    QJniObject pendingIntent = QJniObject::callStaticObjectMethod(
        "android/app/PendingIntent",
        "getBroadcast",
        "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;",
        activity.object<jobject>(),
        0,
        intent.object<jobject>(),
        QJniObject::getStaticField<jint>("android/app/PendingIntent", "FLAG_IMMUTABLE")
    );
    
    // Request permission
    usbManager.callMethod<void>(
        "requestPermission",
        "(Landroid/hardware/usb/UsbDevice;Landroid/app/PendingIntent;)V",
        device.object<jobject>(),
        pendingIntent.object<jobject>()
    );
    
    qDebug() << "Requested USB permission for" << deviceName;
    return false; // Will be granted asynchronously
}

int AndroidUsbHelper::openDevice(const QString& deviceName)
{
    if (!hasPermission(deviceName)) {
        qWarning() << "No permission for device:" << deviceName;
        return -1;
    }
    
    QJniObject usbManager = getUsbManager();
    if (!usbManager.isValid()) {
        return -1;
    }
    
    // Get device by name
    QJniObject jDeviceName = QJniObject::fromString(deviceName);
    QJniObject deviceList = usbManager.callObjectMethod(
        "getDeviceList",
        "()Ljava/util/HashMap;"
    );
    
    QJniObject device = deviceList.callObjectMethod(
        "get",
        "(Ljava/lang/Object;)Ljava/lang/Object;",
        jDeviceName.object<jobject>()
    );
    
    if (!device.isValid()) {
        qWarning() << "Device not found:" << deviceName;
        return -1;
    }
    
    // Open device connection
    QJniObject connection = usbManager.callObjectMethod(
        "openDevice",
        "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;",
        device.object<jobject>()
    );
    
    if (!connection.isValid()) {
        qWarning() << "Failed to open USB device:" << deviceName;
        return -1;
    }
    
    // Get file descriptor
    jint fd = connection.callMethod<jint>("getFileDescriptor");
    
    if (fd <= 0) {
        qWarning() << "Invalid file descriptor:" << fd;
        return -1;
    }
    
    // Duplicate the FD so we can manage its lifetime
    int dupFd = dup(fd);
    
    qDebug() << "Opened USB device" << deviceName << "with FD:" << dupFd;
    return dupFd;
}

void AndroidUsbHelper::closeDevice(int fd)
{
    if (fd > 0) {
        close(fd);
        qDebug() << "Closed USB device FD:" << fd;
    }
}

#endif // Q_OS_ANDROID
