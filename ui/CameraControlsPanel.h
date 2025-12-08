#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QListWidget>
#include <QPushButton>

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

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    void setupUI();

    CameraController* _controller;
    ZoomController* _zoomController;
    QListWidget* _cameraList;
    QList<CameraProfile> _availableCameras;
};

#endif // CAMERACONTROLSPANEL_H
