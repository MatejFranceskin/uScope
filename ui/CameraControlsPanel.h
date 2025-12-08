#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCameraFormat>

class CameraController;
class ZoomController;
class CameraProfile;

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
    void onFormatSelected(int index);
    void onExposureChanged(int value);
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onSaturationChanged(int value);
    void onAutoWhiteBalanceClicked();
    void onFlipHorizontalClicked();
    void onFlipVerticalClicked();
    void onResetDefaultsClicked();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void updateGeometry(float baseTileSize, float sceneWidth, int fontSize = 0) override;

private:
    void setupUI();

    CameraController* _controller;
    ZoomController* _zoomController;
    QComboBox* _cameraCombo;
    QComboBox* _formatCombo;
    QWidget* _contentWidget;
    QList<CameraProfile> _availableCameras;
    QList<QCameraFormat> _availableFormats;
    
    // Camera control widgets
    QSlider* _exposureSlider;
    QSlider* _brightnessSlider;
    QSlider* _contrastSlider;
    QSlider* _saturationSlider;
    QLabel* _exposureLabel;
    QLabel* _brightnessLabel;
    QLabel* _contrastLabel;
    QLabel* _saturationLabel;
    QPushButton* _autoWhiteBalanceBtn;
    QPushButton* _flipHorizontalBtn;
    QPushButton* _flipVerticalBtn;
    QPushButton* _resetDefaultsBtn;
};

#endif // CAMERACONTROLSPANEL_H
