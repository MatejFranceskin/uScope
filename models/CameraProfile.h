#ifndef CAMERAPROFILE_H
#define CAMERAPROFILE_H

#include <QString>
#include <QSize>
#include <QDateTime>
#include <QJsonObject>

/**
 * Camera device profile with configuration and metadata
 * Represents a detected UVC/camera device with supported formats
 */
class CameraProfile
{
public:
    CameraProfile();
    CameraProfile(const QString& id, const QString& name, const QString& manufacturer = QString());

    // Getters
    QString id() const { return _id; }
    QString name() const { return _name; }
    QString manufacturer() const { return _manufacturer; }
    QSize resolution() const { return _resolution; }
    int frameRate() const { return _frameRate; }
    QDateTime lastUsed() const { return _lastUsed; }

    // Setters
    void setId(const QString& id) { _id = id; }
    void setName(const QString& name) { _name = name; }
    void setManufacturer(const QString& manufacturer) { _manufacturer = manufacturer; }
    void setResolution(const QSize& resolution) { _resolution = resolution; }
    void setFrameRate(int frameRate) { _frameRate = frameRate; }
    void setLastUsed(const QDateTime& lastUsed) { _lastUsed = lastUsed; }

    // Validation
    bool isValid() const;

    // Serialization
    QJsonObject toJson() const;
    static CameraProfile fromJson(const QJsonObject& json);

private:
    QString _id;              // Unique camera identifier (e.g., "/dev/video0", "USB\VID_046D...")
    QString _name;            // Display name (e.g., "USB Microscope X200")
    QString _manufacturer;    // Manufacturer name (optional)
    QSize _resolution;        // Current/preferred resolution
    int _frameRate;           // Current/preferred frame rate (fps)
    QDateTime _lastUsed;      // Last time this camera was used
};

#endif // CAMERAPROFILE_H
