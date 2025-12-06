#include "Tile.h"
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneHoverEvent>

Tile::Tile(float widthMultiplier, float heightMultiplier, Anchor anchor, QGraphicsItem* parent)
    : QGraphicsWidget(parent)
    , _widthMultiplier(widthMultiplier)
    , _heightMultiplier(heightMultiplier)
    , _anchor(anchor)
    , _state(TileState::Idle)
    , _currentBaseTileSize(0.0f)
{
    setAcceptHoverEvents(true);
}

void Tile::updateGeometry(float baseTileSize, int positionIndex)
{
    _currentBaseTileSize = baseTileSize;
    
    // Calculate tile dimensions
    float width = baseTileSize * _widthMultiplier;
    float height = baseTileSize * _heightMultiplier;
    
    // Set size
    resize(width, height);
    
    // Calculate position based on anchor and index
    float spacing = baseTileSize * 0.5f;  // 0.5 tile spacing between tiles
    float yPos = spacing + (positionIndex * (height + spacing));
    
    // Position is set by MainWindow based on scene dimensions
    // This method just stores the size for later positioning
    setProperty("yPosition", yPos);
}

float Tile::cornerRadius(float baseTileSize)
{
    return baseTileSize / 8.0f;
}

void Tile::setState(TileState state)
{
    if (_state != state) {
        _state = state;
        update();  // Trigger repaint
    }
}

void Tile::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Background color based on state
    QColor bgColor;
    switch (_state) {
        case TileState::Idle:
            bgColor = QColor(40, 40, 40, 200);  // Dark semi-transparent
            break;
        case TileState::Hover:
            bgColor = QColor(60, 60, 60, 220);  // Lighter on hover
            break;
        case TileState::Active:
            bgColor = QColor(0, 120, 215, 220);  // Qt blue for active
            break;
        case TileState::Disabled:
            bgColor = QColor(30, 30, 30, 150);  // Darker, more transparent
            break;
    }
    
    // Draw rounded rectangle background
    float radius = cornerRadius(_currentBaseTileSize);
    painter->setBrush(bgColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect(), radius, radius);
    
    // Subclasses will override to add content (icons, text, etc.)
}

void Tile::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    if (_state == TileState::Idle) {
        setState(TileState::Hover);
    }
}

void Tile::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    if (_state == TileState::Hover) {
        setState(TileState::Idle);
    }
}
