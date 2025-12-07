#include "ZoomTile.h"
#include "../controllers/ZoomController.h"
#include "../models/ZoomState.h"
#include "../TileButton.h"
#include <QPainter>
#include <QGraphicsScene>

ZoomTile::ZoomTile(ZoomController* zoomController, QGraphicsItem* parent)
    : TileButton("", "Zoom", 1.0f, 1.0f, Tile::Anchor::Right, 0, 6, parent)
    , _zoomController(zoomController)
    , _displayText("Fit W")
    , _oneToOneButton(nullptr)
    , _fitWidthButton(nullptr)
    , _fitHeightButton(nullptr)
    , _panelVisible(false)
{
    // Connect to zoom state changes
    connect(_zoomController->zoomState(), &ZoomState::stateChanged,
            this, &ZoomTile::onZoomStateChanged);
    
    // Connect to zoom mode changes to detect external changes (wheel/gesture)
    connect(_zoomController, &ZoomController::zoomChanged,
            this, [this](qreal factor, int mode) {
                Q_UNUSED(factor)
                // If mode changed to Custom (from wheel/gesture), close panel
                if (mode == static_cast<int>(ZoomState::Mode::Custom) && _panelVisible) {
                    hidePanel();
                }
            });
    
    // Override the default clicked connection
    disconnect(this, &TileButton::clicked, nullptr, nullptr);
    connect(this, &TileButton::clicked, this, &ZoomTile::onZoomTileClicked);
    
    // Initialize display text
    onZoomStateChanged();
    
    // Create panel buttons (initially hidden)
    createPanelButtons();
}

ZoomTile::~ZoomTile()
{
    // Buttons deleted automatically via parent ownership
}

void ZoomTile::createPanelButtons()
{
    // Create panel buttons positioned below the zoom tile
    // Right, 1,6 (one column to the right for sub-tiles)
    
    _oneToOneButton = new TileButton("", "1:1", 1.0f, 1.0f, Tile::Anchor::Right, 1, 6);
    _oneToOneButton->setParentItem(nullptr);  // Will be added to scene by MainWindow
    _oneToOneButton->hide();
    connect(_oneToOneButton, &TileButton::clicked, this, &ZoomTile::onOneToOneClicked);
    
    _fitWidthButton = new TileButton("", "Fit W", 1.0f, 1.0f, Tile::Anchor::Right, 2, 6);
    _fitWidthButton->setParentItem(nullptr);
    _fitWidthButton->hide();
    connect(_fitWidthButton, &TileButton::clicked, this, &ZoomTile::onFitWidthClicked);
    
    _fitHeightButton = new TileButton("", "Fit H", 1.0f, 1.0f, Tile::Anchor::Right, 3, 6);
    _fitHeightButton->setParentItem(nullptr);
    _fitHeightButton->hide();
    connect(_fitHeightButton, &TileButton::clicked, this, &ZoomTile::onFitHeightClicked);
}

void ZoomTile::updateGeometry(float baseTileSize, float sceneWidth)
{
    // Update zoom tile geometry
    TileButton::updateGeometry(baseTileSize, sceneWidth);
    
    // Update panel buttons geometry if they exist
    if (_oneToOneButton) {
        _oneToOneButton->updateGeometry(baseTileSize, sceneWidth);
    }
    if (_fitWidthButton) {
        _fitWidthButton->updateGeometry(baseTileSize, sceneWidth);
    }
    if (_fitHeightButton) {
        _fitHeightButton->updateGeometry(baseTileSize, sceneWidth);
    }
}

void ZoomTile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Call base class to paint button background
    TileButton::paint(painter, option, widget);
    
    // Paint zoom display text in the center
    painter->setRenderHint(QPainter::Antialiasing, true);
    
    QRectF bounds = boundingRect();
    QFont font = painter->font();
    font.setPixelSize(bounds.height() * 0.25);
    font.setBold(true);
    painter->setFont(font);
    
    // Use white text
    painter->setPen(Qt::white);
    painter->drawText(bounds, Qt::AlignCenter, _displayText);
}

void ZoomTile::onZoomTileClicked()
{
    if (_panelVisible) {
        hidePanel();
    } else {
        showPanel();
    }
}

void ZoomTile::onZoomStateChanged()
{
    // Update display text from zoom state
    _displayText = _zoomController->zoomState()->displayString();
    update();
}

void ZoomTile::onOneToOneClicked()
{
    _zoomController->setOneToOne();
    hidePanel();
}

void ZoomTile::onFitWidthClicked()
{
    _zoomController->setFitWidth();
    hidePanel();
}

void ZoomTile::onFitHeightClicked()
{
    _zoomController->setFitHeight();
    hidePanel();
}

void ZoomTile::showPanel()
{
    if (_oneToOneButton && _fitWidthButton && _fitHeightButton) {
        // Add buttons to scene if not already added
        if (!_oneToOneButton->scene()) {
            scene()->addItem(_oneToOneButton);
        }
        if (!_fitWidthButton->scene()) {
            scene()->addItem(_fitWidthButton);
        }
        if (!_fitHeightButton->scene()) {
            scene()->addItem(_fitHeightButton);
        }
        
        _oneToOneButton->show();
        _fitWidthButton->show();
        _fitHeightButton->show();
        _panelVisible = true;
        
        // Set tile to active state
        setState(TileState::Active);
    }
}

void ZoomTile::hidePanel()
{
    if (_oneToOneButton && _fitWidthButton && _fitHeightButton) {
        _oneToOneButton->hide();
        _fitWidthButton->hide();
        _fitHeightButton->hide();
        _panelVisible = false;
        
        // Return tile to normal state
        setState(TileState::Idle);
    }
}
