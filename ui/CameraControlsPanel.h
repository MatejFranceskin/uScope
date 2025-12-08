#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QListWidget>
#include <QPushButton>
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
};

#endif // CAMERACONTROLSPANEL_H
