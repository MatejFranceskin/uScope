#ifndef ANDROIDUSBHELPER_H
#define ANDROIDUSBHELPER_H

#include <QObject>
#include <QString>
#include <QList>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
#include <QtAndroid>
#else
// Qt 6.2+ has JNI functionality in QtCore
#endif
#endif

/**
 * @brief Helper class for Android USB device access
 * 
 * Provides JNI interface to Android's UsbManager for:
 * - Enumerating USB devices
 * - Requesting USB permissions
 * - Opening USB devices and obtaining file descriptors
 */
class AndroidUsbHelper : public QObject
{
    Q_OBJECT

public:
    struct UsbDeviceInfo {
        int vendorId;
        int productId;
        QString deviceName;
        QString manufacturer;
        QString product;
    };

    explicit AndroidUsbHelper(QObject *parent = nullptr);
    ~AndroidUsbHelper();

#ifdef Q_OS_ANDROID
    /**
     * @brief Get list of connected USB devices
     */
    static QList<UsbDeviceInfo> getUsbDevices();

    /**
     * @brief Request permission for USB device
     * @param deviceName Device name from UsbDevice.getDeviceName()
     * @return true if permission granted or already has permission
     */
    static bool requestPermission(const QString& deviceName);

    /**
     * @brief Check if we have permission for a device
     */
    static bool hasPermission(const QString& deviceName);

    /**
     * @brief Open USB device and get file descriptor
     * @param deviceName Device name from UsbDevice.getDeviceName()
     * @return File descriptor, or -1 on error
     */
    static int openDevice(const QString& deviceName);

    /**
     * @brief Close file descriptor
     */
    static void closeDevice(int fd);

signals:
    void permissionGranted(const QString& deviceName);
    void permissionDenied(const QString& deviceName);

private:
    static QJniObject getUsbManager();
#endif // Q_OS_ANDROID
};

#endif // ANDROIDUSBHELPER_H
