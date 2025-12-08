#include "CameraControlsPanel.h"
#include "../controllers/CameraController.h"
#include <QPainter>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

CameraControlsPanel::CameraControlsPanel(CameraController* controller, QGraphicsItem* parent)
    : TileDialog(6.0f, 6.0f, 1, parent)  // 6×6 tiles, positioned at Y=1 from top
    , _controller(controller)
    , _cameraList(nullptr)
{
    setupUI();
}

void CameraControlsPanel::setupUI()
{
    // Create content widget with standard Qt controls
    QWidget* content = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // Camera selection label
    QLabel* cameraLabel = new QLabel("Select Camera:");
    
    // Camera list widget
    _cameraList = new QListWidget();
    _cameraList->setMinimumHeight(150);
    _cameraList->setStyleSheet(
        "QListWidget {"
        "   background-color: rgba(60, 60, 60, 200);"
        "   color: white;"
        "   border: 2px solid rgba(100, 100, 100, 200);"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(80, 150, 80, 220);"
        "}"
        "QListWidget::item:hover {"
        "   background-color: rgba(120, 120, 150, 200);"
        "}"
    );
    connect(_cameraList, &QListWidget::currentRowChanged,
            this, &CameraControlsPanel::onCameraSelected);
    
    // Assemble layout
    layout->addWidget(cameraLabel);
    layout->addWidget(_cameraList);
    
    // Set as dialog content - scroll bars appear automatically if needed
    setContentWidget(content);
    
    // Initial camera refresh
    refreshCameras();
}

void CameraControlsPanel::refreshCameras()
{
    _availableCameras = _controller->availableCameras();
    
    _cameraList->clear();
    
    if (_availableCameras.isEmpty()) {
        _cameraList->addItem("No cameras detected");
    } else {
        for (const CameraProfile& camera : _availableCameras) {
            QListWidgetItem* item = new QListWidgetItem(camera.name());
            item->setData(Qt::UserRole, camera.id());
            _cameraList->addItem(item);
        }
        _cameraList->setCurrentRow(0);
    }
}

void CameraControlsPanel::onCameraSelected(int index)
{
    if (index >= 0 && index < _availableCameras.size()) {
        QString cameraId = _availableCameras[index].id();
        _controller->startCamera(cameraId);
    }
}

void CameraControlsPanel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Draw base dialog background with title
    TileDialog::paint(painter, option, widget);
}
