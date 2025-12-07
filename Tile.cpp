#include "Tile.h"
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneHoverEvent>

Tile::Tile(float widthMultiplier, float heightMultiplier, Anchor anchor, int gridX, int gridY, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , _widthMultiplier(widthMultiplier)
    , _heightMultiplier(heightMultiplier)
    , _anchor(anchor)
    , _gridX(gridX)
    , _gridY(gridY)
    , _state(TileState::Idle)
    , _currentBaseTileSize(0.0f)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    
    // Try ItemCoordinateCache instead - less aggressive but might work better
    setCacheMode(QGraphicsItem::ItemCoordinateCache);
}

void Tile::updateGeometry(float baseTileSize, float sceneWidth)
{
    _currentBaseTileSize = baseTileSize;
    
    // baseTileSize includes both the tile and its margin
    // Margin is 10% of baseTileSize (top/bottom or left/right)
    float margin = baseTileSize * 0.1f;
    
    // Calculate actual tile dimensions (baseTileSize minus margins)
    float tileSize = baseTileSize - margin;
    float width = tileSize * _widthMultiplier;
    float height = tileSize * _heightMultiplier;
    
    // Store dimensions for boundingRect
    _width = width;
    _height = height;
    
    // Calculate position based on anchor and grid coordinates
    float xPos = 0.0f;
    float yPos = _gridY * baseTileSize + margin;  // Y grid position * baseTileSize + top margin
    
    switch (_anchor) {
        case Anchor::Left:
            // X=0 is leftmost, increases rightward
            xPos = margin + (_gridX * baseTileSize);
            break;
            
        case Anchor::Right:
            // X=0 is rightmost (closest to edge), X increases leftward
            xPos = sceneWidth - width - margin - (_gridX * baseTileSize);
            break;
            
        case Anchor::Center:
            // X=0 is center, positive = right, negative = left
            xPos = (sceneWidth - width) / 2.0f + (_gridX * baseTileSize);
            break;
    }
    
    setPos(xPos, yPos);
    // Don't call prepareGeometryChange() - we're only changing position, not size
    // Calling it invalidates the item cache causing flickering
}

float Tile::cornerRadius(float baseTileSize)
{
    return baseTileSize / 8.0f;
}

QRectF Tile::boundingRect() const
{
    return QRectF(0, 0, _width, _height);
}

void Tile::setState(TileState state)
{
    if (_state != state) {
        _state = state;
        update(boundingRect());  // Update only the bounding rect area
    }
}

QPainterPath Tile::shape() const
{
    // Return the rounded rectangle shape for proper hit testing and clipping
    QPainterPath path;
    float radius = cornerRadius(_currentBaseTileSize);
    path.addRoundedRect(boundingRect(), radius, radius);
    return path;
}

void Tile::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Background color based on state - semi-transparent for overlay effect
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
