#include "CapturedImage.h"
#include <QStandardPaths>
#include <QDir>
#include <QImageWriter>

CapturedImage::CapturedImage()
    : _timestamp(QDateTime::currentDateTime())
{
}

CapturedImage::CapturedImage(const QImage& imageData, const QString& cameraId)
    : _timestamp(QDateTime::currentDateTime())
    , _cameraId(cameraId)
{
    setImageData(imageData);
}

void CapturedImage::setImageData(const QImage& imageData)
{
    _imageData = imageData;
    if (!imageData.isNull()) {
        _resolution = imageData.size();
    }
}

bool CapturedImage::isValid() const
{
    return !_imageData.isNull() 
        && _resolution.width() > 0 
        && _resolution.height() > 0;
}

QString CapturedImage::generateFilename() const
{
    // Format: uScope_YYYYMMDD_HHmmss.jpg
    return QString("uScope_%1.jpg").arg(_timestamp.toString("yyyyMMdd_HHmmss"));
}

bool CapturedImage::save(const QString& directory)
{
    if (!isValid()) {
        return false;
    }

    // Determine save directory
    QString saveDir = directory;
    if (saveDir.isEmpty()) {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        saveDir = documentsPath + QDir::separator() + "uScope";
    }

    // Create directory if it doesn't exist
    QDir dir(saveDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            return false;
        }
    }

    // Generate filename and full path
    QString filename = generateFilename();
    _filePath = saveDir + QDir::separator() + filename;

    // Save image with metadata
    QImageWriter writer(_filePath, "JPEG");
    writer.setQuality(95);  // High quality JPEG (0-100)
    
    // Add metadata as EXIF comments
    writer.setText("Camera ID", _cameraId);
    writer.setText("Timestamp", _timestamp.toString(Qt::ISODate));
    writer.setText("Resolution", QString("%1x%2").arg(_resolution.width()).arg(_resolution.height()));
    writer.setText("Software", "uScope Microscopy Platform");

    if (!writer.write(_imageData)) {
        _filePath.clear();
        return false;
    }

    return true;
}
