#ifndef CAMERACONTROLSPANEL_H
#define CAMERACONTROLSPANEL_H

#include "../TileDialog.h"
#include <QList>
#include <QListWidget>
#include <QPushButton>

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

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    void setupUI();

    CameraController* _controller;
    QListWidget* _cameraList;
    QList<CameraProfile> _availableCameras;
};

#endif // CAMERACONTROLSPANEL_H
