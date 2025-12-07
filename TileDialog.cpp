#include "TileDialog.h"
#include <QGraphicsScene>
#include <QKeyEvent>

TileDialog::TileDialog(float widthMultiplier, float heightMultiplier, int gridY, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, Anchor::Center, 0, gridY, parent)
    , _dimOverlay(nullptr)
{
    // Dialogs start hidden
    QGraphicsItem::setVisible(false);
}

TileDialog::~TileDialog()
{
    if (_dimOverlay) {
        delete _dimOverlay;
    }
}

void TileDialog::show(float baseTileSize, float sceneWidth)
{
    if (!scene()) {
        return;
    }
    
    // Create dim overlay if it doesn't exist
    if (!_dimOverlay) {
        _dimOverlay = new QGraphicsRectItem();
        _dimOverlay->setBrush(QColor(0, 0, 0, 180));  // Semi-transparent black
        _dimOverlay->setPen(Qt::NoPen);
        _dimOverlay->setZValue(-1);  // Behind dialog but above everything else
        scene()->addItem(_dimOverlay);
    }
    
    // Size overlay to cover entire scene
    QRectF sceneRect = scene()->sceneRect();
    _dimOverlay->setRect(sceneRect);
    _dimOverlay->setVisible(true);
    
    // Update dialog geometry (this also updates children)
    updateGeometry(baseTileSize, sceneWidth);
    
    // Bring dialog to front
    setZValue(1000);
    
    // Show dialog
    QGraphicsItem::setVisible(true);
    setFocus();
}

void TileDialog::hide()
{
    QGraphicsItem::setVisible(false);
    
    if (_dimOverlay) {
        _dimOverlay->setVisible(false);
    }
}

bool TileDialog::isVisible() const
{
    return QGraphicsItem::isVisible();
}

void TileDialog::updateGeometry(float baseTileSize, float sceneWidth)
{
    // Update dialog's own geometry
    Tile::updateGeometry(baseTileSize, sceneWidth);
    
    // Update dim overlay size if it exists (regardless of visibility)
    if (_dimOverlay && scene()) {
        _dimOverlay->setRect(scene()->sceneRect());
    }
    
    // Update all child tiles
    for (QGraphicsItem* child : childItems()) {
        if (Tile* tile = dynamic_cast<Tile*>(child)) {
            tile->updateGeometry(baseTileSize, _width);  // Use dialog width as scene width for children
        }
    }
}

void TileDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        emit rejected();
        event->accept();
    } else {
        Tile::keyPressEvent(event);
    }
}

void TileDialog::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    // Draw the dialog background (lighter than normal tiles)
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = boundingRect();
    float radius = cornerRadius(_currentBaseTileSize);
    
    // Dialog background - lighter, more opaque
    painter->setBrush(QColor(60, 60, 60, 240));
    painter->setPen(QPen(QColor(100, 100, 100), 2));
    painter->drawRoundedRect(rect, radius, radius);
}
