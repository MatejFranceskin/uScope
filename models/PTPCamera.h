#pragma once

#include <QString>
#include <QMap>
#include <QVariant>

enum class ConnectionType {
    USB,
    Network
};

class PTPCamera
{
public:
    PTPCamera();
    PTPCamera(const QString& id, const QString& manufacturer, const QString& model);
    
    // Getters
    QString id() const { return _id; }
    QString manufacturer() const { return _manufacturer; }
    QString model() const { return _model; }
    QString serialNumber() const { return _serialNumber; }
    ConnectionType connectionType() const { return _connectionType; }
    
    QMap<QString, QVariant> capabilities() const { return _capabilities; }
    QMap<QString, QVariant> currentSettings() const { return _currentSettings; }
    
    // Setters
    void setSerialNumber(const QString& serialNumber) { _serialNumber = serialNumber; }
    void setConnectionType(ConnectionType type) { _connectionType = type; }
    
    void setCapabilities(const QMap<QString, QVariant>& capabilities);
    void setCurrentSettings(const QMap<QString, QVariant>& settings) { _currentSettings = settings; }
    void updateSetting(const QString& name, const QVariant& value);
    
    // Validation
    bool isSettingValid(const QString& name, const QVariant& value) const;
    QStringList getValidValues(const QString& name) const;

private:
    QString _id;
    QString _manufacturer;
    QString _model;
    QString _serialNumber;
    ConnectionType _connectionType;
    
    QMap<QString, QVariant> _capabilities;
    QMap<QString, QVariant> _currentSettings;
    
    // Helper methods
    void parseCapabilities();
    bool isManualOrAperturePriority(const QString& mode) const;
};
