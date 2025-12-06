#ifndef CAPTUREDIMAGE_H
#define CAPTUREDIMAGE_H

#include <QString>
#include <QDateTime>
#include <QSize>
#include <QImage>

/**
 * Captured microscopy image with metadata
 * Represents a snapshot from the camera with timestamp and camera info
 */
class CapturedImage
{
public:
    CapturedImage();
    CapturedImage(const QImage& imageData, const QString& cameraId);

    // Getters
    QString filePath() const { return _filePath; }
    QDateTime timestamp() const { return _timestamp; }
    QSize resolution() const { return _resolution; }
    QString cameraId() const { return _cameraId; }
    QImage imageData() const { return _imageData; }

    // Setters
    void setFilePath(const QString& filePath) { _filePath = filePath; }
    void setCameraId(const QString& cameraId) { _cameraId = cameraId; }
    void setImageData(const QImage& imageData);

    // Operations
    bool save(const QString& directory = QString());
    bool isValid() const;

private:
    QString generateFilename() const;

    QString _filePath;        // Full path to saved file
    QDateTime _timestamp;     // Capture timestamp
    QSize _resolution;        // Image resolution
    QString _cameraId;        // Camera that captured this image
    QImage _imageData;        // Image pixel data
};

#endif // CAPTUREDIMAGE_H
