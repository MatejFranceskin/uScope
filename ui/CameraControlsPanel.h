#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>

class TileCombo;
class TileButton;
class CameraController;
class CameraProfile;

/**
 * Camera controls panel as modal tile dialog
 * Provides camera selection and capture button
 */
class CameraControlsPanel : public TileDialog
{
    Q_OBJECT

public:
    explicit CameraControlsPanel(CameraController* controller, QGraphicsItem* parent = nullptr);

public slots:
    void refreshCameras();

signals:
    void captureRequested();

private slots:
    void onCameraSelected(int index);
    void onCaptureClicked();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    void setupUI();

    CameraController* _controller;
    TileCombo* _cameraSelector;
    TileButton* _captureButton;
    QList<CameraProfile> _availableCameras;
};

#endif // CAMERACONTROLSPANEL_H
