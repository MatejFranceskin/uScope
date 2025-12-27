#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QSize>

class CameraController;
class ZoomController;
class CameraProfile;
class QGridLayout;

/**
 * Camera controls panel as modal tile dialog
 * Provides camera selection and capture button
 */
class CameraControlsPanel : public TileDialog
{
    Q_OBJECT

public:
    explicit CameraControlsPanel(CameraController* cameraController, 
                                 ZoomController* zoomController,
                                 QGraphicsItem* parent = nullptr);

    void show(float baseTileSize, float sceneWidth) override;

public slots:
    void refreshCameras();

signals:
    void captureRequested();

private slots:
    void onCameraSelected(int index);
    void onResolutionSelected(int index);
    void onFpsSelected(int index);
    void onExposureChanged(int value);
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onSaturationChanged(int value);
    void onWhiteBalanceChanged(int value);
    void onAutoExposureClicked();
    void onAutoWhiteBalanceClicked();
    void onFlipHorizontalClicked();
    void onFlipVerticalClicked();
    void onResetDefaultsClicked();
    
    // PTP camera control slots
    void onPTPExposureModeChanged(int index);
    void onPTPIsoChanged(int index);
    void onPTPShutterSpeedChanged(int index);
    void onPTPExposureCompChanged(int index);
    void onPTPWhiteBalanceChanged(int index);


protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void updateGeometry(float baseTileSize, float sceneWidth, int fontSize = 0) override;

private:
    void setupUI();
    void setupUVCControls(QGridLayout* layout, int& row);
    void setupPTPControls(QGridLayout* layout, int& row);
    void updateControlsVisibility();

    CameraController* _controller;
    ZoomController* _zoomController;
    QComboBox* _cameraCombo;
    QComboBox* _resolutionCombo;
    QComboBox* _fpsCombo;
    QLabel* _resolutionLabel;
    QLabel* _fpsLabel;
    QWidget* _contentWidget;
    QList<CameraProfile> _availableCameras;
    QList<QSize> _availableResolutions;
    QList<double> _availableFrameRates;
    
    // UVC Camera control widgets
    QWidget* _uvcControlsWidget;
    QSlider* _exposureSlider;
    QSlider* _brightnessSlider;
    QSlider* _contrastSlider;
    QSlider* _saturationSlider;
    QSlider* _whiteBalanceSlider;
    QLabel* _exposureLabel;
    QLabel* _brightnessLabel;
    QLabel* _contrastLabel;
    QLabel* _saturationLabel;
    QLabel* _whiteBalanceLabel;
    QLabel* _exposureValueLabel;
    QLabel* _brightnessValueLabel;
    QLabel* _contrastValueLabel;
    QLabel* _saturationValueLabel;
    QLabel* _whiteBalanceValueLabel;
    QCheckBox* _autoExposureCheck;
    QCheckBox* _autoWhiteBalanceCheck;
    QPushButton* _flipHorizontalBtn;
    QPushButton* _flipVerticalBtn;
    QPushButton* _resetDefaultsBtn;
    bool _autoExposureEnabled;
    bool _autoWhiteBalanceEnabled;
    
    // PTP Camera control widgets
    QWidget* _ptpControlsWidget;
    QComboBox* _ptpExposureModeCombo;
    QComboBox* _ptpIsoCombo;
    QComboBox* _ptpShutterSpeedCombo;
    QComboBox* _ptpExposureCompCombo;
    QComboBox* _ptpWhiteBalanceCombo;

    QLabel* _ptpExposureModeLabel;
    QLabel* _ptpIsoLabel;
    QLabel* _ptpShutterSpeedLabel;
    QLabel* _ptpExposureCompLabel;
    QLabel* _ptpWhiteBalanceLabel;

};

#endif // CAMERACONTROLSPANEL_H
