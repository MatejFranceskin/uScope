#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
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

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void updateGeometry(float baseTileSize, float sceneWidth, int fontSize = 0) override;

private:
    void setupUI();

    CameraController* _controller;
    ZoomController* _zoomController;
    QListWidget* _cameraList;
    QListWidget* _formatList;
    QWidget* _contentWidget;
    QList<CameraProfile> _availableCameras;
    QList<QCameraFormat> _availableFormats;
    
    // Camera control widgets
    QSlider* _exposureSlider;
    QSlider* _brightnessSlider;
    QSlider* _contrastSlider;
    QSlider* _saturationSlider;
    QPushButton* _autoWhiteBalanceBtn;
    QPushButton* _flipHorizontalBtn;
    QPushButton* _flipVerticalBtn;
};

#endif // CAMERACONTROLSPANEL_H
