#include "CameraControlsPanel.h"
#include "../TileCombo.h"
#include "../TileButton.h"
#include "../TileLabel.h"
#include "../controllers/CameraController.h"
#include <QPainter>
#include <QFont>

CameraControlsPanel::CameraControlsPanel(CameraController* controller, QGraphicsItem* parent)
    : TileDialog(6.0f, 4.0f, 1, parent)  // 6×4 tiles, positioned at Y=1 from top
    , _controller(controller)
    , _cameraSelector(nullptr)
    , _captureButton(nullptr)
    , _okButton(nullptr)
    , _cancelButton(nullptr)
{
    setupUI();
}

void CameraControlsPanel::setupUI()
{
    // All positions are in dialog's local grid coordinates
    // Dialog is 6 tiles wide × 4 tiles tall
    
    // Camera selector combo - centered horizontally at top (Y=0)
    _cameraSelector = new TileCombo(4.0f, 1.0f, Tile::Anchor::Center, 0, 0, this);
    connect(_cameraSelector, QOverload<int>::of(&TileCombo::currentIndexChanged),
            this, &CameraControlsPanel::onCameraSelected);
    
    // Capture button removed - user will select camera and click OK
    
    // OK button - left side at bottom (Y=3)
    _okButton = new TileButton("", "OK", 2.0f, 1.0f, Tile::Anchor::Center, -1, 3, this);
    connect(_okButton, &TileButton::clicked, this, &CameraControlsPanel::onOkClicked);
    
    // Cancel button - right side at bottom (Y=3)
    _cancelButton = new TileButton("", "Cancel", 2.0f, 1.0f, Tile::Anchor::Center, 1, 3, this);
    connect(_cancelButton, &TileButton::clicked, this, &CameraControlsPanel::onCancelClicked);
    
    // Initial camera refresh
    refreshCameras();
}

void CameraControlsPanel::refreshCameras()
{
    _availableCameras = _controller->availableCameras();
    
    _cameraSelector->comboBox()->clear();
    
    if (_availableCameras.isEmpty()) {
        _cameraSelector->addItem("No cameras detected");
        _okButton->setState(TileState::Disabled);
    } else {
        for (const CameraProfile& camera : _availableCameras) {
            _cameraSelector->addItem(camera.name(), camera.id());
        }
        _okButton->setState(TileState::Idle);
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

void CameraControlsPanel::onOkClicked()
{
    // Camera already started by onCameraSelected
    hide();
    emit accepted();
}

void CameraControlsPanel::onCancelClicked()
{
    hide();
    emit rejected();
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
    painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, "Select Camera");
    
    // Position child tiles
    if (_cameraSelector) {
        _cameraSelector->setPos(rect.width() * 0.1f, rect.height() * 0.15f);
    }
    
    if (_captureButton) {
        _captureButton->setPos(rect.width() * 0.25f, rect.height() * 0.3f);
    }
}
