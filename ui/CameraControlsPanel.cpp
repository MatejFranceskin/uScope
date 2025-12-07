#include "CameraControlsPanel.h"
#include "../TileCombo.h"
#include "../TileButton.h"
#include "../TileLabel.h"
#include "../controllers/CameraController.h"
#include <QPainter>
#include <QFont>

CameraControlsPanel::CameraControlsPanel(CameraController* controller, QGraphicsItem* parent)
    : TileDialog(3.0f, 4.0f, parent)
    , _controller(controller)
    , _cameraSelector(nullptr)
    , _captureButton(nullptr)
{
    setupUI();
}

void CameraControlsPanel::setupUI()
{
    // Create camera selector combo (centered, grid position 0,0)
    _cameraSelector = new TileCombo(2.5f, 0.8f, Tile::Anchor::Center, 0, 0, this);
    connect(_cameraSelector, QOverload<int>::of(&TileCombo::currentIndexChanged),
            this, &CameraControlsPanel::onCameraSelected);
    
    // Create capture button (centered, grid position 0,1)
    _captureButton = new TileButton(":/images/media-record.svg", "Snap", 2.0f, 1.2f, Tile::Anchor::Center, 0, 1, this);
    connect(_captureButton, &TileButton::clicked, this, &CameraControlsPanel::onCaptureClicked);
    
    // Initial camera refresh
    refreshCameras();
}

void CameraControlsPanel::refreshCameras()
{
    _availableCameras = _controller->availableCameras();
    
    _cameraSelector->comboBox()->clear();
    
    if (_availableCameras.isEmpty()) {
        _cameraSelector->addItem("No cameras detected");
        _captureButton->setState(TileState::Disabled);
    } else {
        for (const CameraProfile& camera : _availableCameras) {
            _cameraSelector->addItem(camera.name(), camera.id());
        }
        _captureButton->setState(TileState::Idle);
    }
}

void CameraControlsPanel::onCameraSelected(int index)
{
    if (index >= 0 && index < _availableCameras.size()) {
        QString cameraId = _availableCameras[index].id();
        _controller->startCamera(cameraId);
    }
}

void CameraControlsPanel::onCaptureClicked()
{
    emit captureRequested();
}

void CameraControlsPanel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Draw base dialog background
    TileDialog::paint(painter, option, widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = boundingRect();
    
    // Draw title
    painter->setPen(Qt::white);
    QFont titleFont = painter->font();
    titleFont.setPixelSize(static_cast<int>(rect.height() * 0.06f));
    titleFont.setBold(true);
    painter->setFont(titleFont);
    
    QRectF titleRect(rect.left() + 20, rect.top() + 20, rect.width() - 40, rect.height() * 0.1f);
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, "Camera Controls");
    
    // Position child tiles
    if (_cameraSelector) {
        _cameraSelector->setPos(rect.width() * 0.1f, rect.height() * 0.15f);
    }
    
    if (_captureButton) {
        _captureButton->setPos(rect.width() * 0.25f, rect.height() * 0.3f);
    }
}
