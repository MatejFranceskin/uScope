#include "CameraProfile.h"

CameraProfile::CameraProfile()
    : _frameRate(30)
{
}

CameraProfile::CameraProfile(const QString& id, const QString& name, const QString& manufacturer)
    : _id(id)
    , _name(name)
    , _manufacturer(manufacturer)
    , _frameRate(30)
{
}

bool CameraProfile::isValid() const
{
    return !_id.isEmpty() 
        && !_name.isEmpty() 
        && _resolution.width() > 0 
        && _resolution.height() > 0 
        && _frameRate > 0;
}

QJsonObject CameraProfile::toJson() const
{
    QJsonObject json;
    json["id"] = _id;
    json["name"] = _name;
    json["manufacturer"] = _manufacturer;
    json["resolutionWidth"] = _resolution.width();
    json["resolutionHeight"] = _resolution.height();
    json["frameRate"] = _frameRate;
    json["lastUsed"] = _lastUsed.toString(Qt::ISODate);
    return json;
}

CameraProfile CameraProfile::fromJson(const QJsonObject& json)
{
    CameraProfile profile;
    profile._id = json["id"].toString();
    profile._name = json["name"].toString();
    profile._manufacturer = json["manufacturer"].toString();
    profile._resolution = QSize(json["resolutionWidth"].toInt(), json["resolutionHeight"].toInt());
    profile._frameRate = json["frameRate"].toInt();
    profile._lastUsed = QDateTime::fromString(json["lastUsed"].toString(), Qt::ISODate);
    return profile;
}
