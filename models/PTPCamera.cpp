#include "PTPCamera.h"

PTPCamera::PTPCamera()
    : _connectionType(ConnectionType::USB)
{
}

PTPCamera::PTPCamera(const QString& id, const QString& manufacturer, const QString& model)
    : _id(id)
    , _manufacturer(manufacturer)
    , _model(model)
    , _connectionType(ConnectionType::USB)
{
}

void PTPCamera::setCapabilities(const QMap<QString, QVariant>& capabilities)
{
    _capabilities = capabilities;
    parseCapabilities();
}

void PTPCamera::updateSetting(const QString& name, const QVariant& value)
{
    if (isSettingValid(name, value)) {
        _currentSettings[name] = value;
    }
}

bool PTPCamera::isSettingValid(const QString& name, const QVariant& value) const
{
    if (!_capabilities.contains(name + "s") && !_capabilities.contains(name + "Values")) {
        // Setting not in capabilities
        return false;
    }
    
    QStringList validValues = getValidValues(name);
    if (validValues.isEmpty()) {
        // No restriction on values
        return true;
    }
    
    QString valueStr = value.toString();
    
    // Special handling for exposure modes - only allow Manual and Aperture Priority
    if (name == "exposureMode" || name == "expprogram") {
        return isManualOrAperturePriority(valueStr);
    }
    
    return validValues.contains(valueStr);
}

QStringList PTPCamera::getValidValues(const QString& name) const
{
    // Try different capability key variations
    if (_capabilities.contains(name + "s")) {
        return _capabilities[name + "s"].toStringList();
    }
    
    if (_capabilities.contains(name + "Values")) {
        return _capabilities[name + "Values"].toStringList();
    }
    
    // Handle specific known mappings
    if (name == "iso" && _capabilities.contains("isoValues")) {
        return _capabilities["isoValues"].toStringList();
    }
    
    if (name == "shutterspeed" && _capabilities.contains("shutterSpeeds")) {
        return _capabilities["shutterSpeeds"].toStringList();
    }
    
    if (name == "aperture" && _capabilities.contains("apertureValues")) {
        return _capabilities["apertureValues"].toStringList();
    }
    
    if (name == "expprogram" && _capabilities.contains("exposureModes")) {
        return _capabilities["exposureModes"].toStringList();
    }
    
    return QStringList();
}

void PTPCamera::parseCapabilities()
{
    // Filter exposure modes to only Manual and Aperture Priority
    if (_capabilities.contains("exposureModes")) {
        QStringList modes = _capabilities["exposureModes"].toStringList();
        QStringList filteredModes;
        
        for (const QString& mode : modes) {
            if (isManualOrAperturePriority(mode)) {
                filteredModes.append(mode);
            }
        }
        
        _capabilities["exposureModes"] = filteredModes;
    }
}

bool PTPCamera::isManualOrAperturePriority(const QString& mode) const
{
    QString modeLower = mode.toLower();
    
    // Match various representations of Manual mode
    if (modeLower.contains("manual") || modeLower == "m") {
        return true;
    }
    
    // Match various representations of Aperture Priority mode
    if (modeLower.contains("aperture") || modeLower == "a" || modeLower == "av") {
        return true;
    }
    
    return false;
}
