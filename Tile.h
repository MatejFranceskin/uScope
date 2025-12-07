#ifndef TILE_H
#define TILE_H

#include <QGraphicsObject>
#include <QPainter>

/**
 * Base class for tile-based UI overlay components
 * 
 * Tiles use proportional sizing based on baseTileSize = screenHeight / 12
 * and support anchoring to Left, Right, or Center screen positions.
 */
class Tile : public QGraphicsObject
{
    Q_OBJECT

public:
    enum class Anchor {
        Left,
        Right,
        Center
    };

    enum class TileState {
        Idle,
        Hover,
        Active,
        Disabled
    };

    /**
     * Construct a tile with size multipliers, anchor position, and grid coordinates
     * @param widthMultiplier Multiplier for baseTileSize to determine width
     * @param heightMultiplier Multiplier for baseTileSize to determine height
     * @param anchor Screen position anchor (Left, Right, Center)
     * @param gridX X position in grid (meaning depends on anchor)
     * @param gridY Y position in grid (0 = top, always from top down)
     * @param parent Parent graphics item
     */
    Tile(float widthMultiplier, float heightMultiplier, Anchor anchor, int gridX, int gridY, QGraphicsItem* parent = nullptr);

    /**
     * Update tile geometry based on baseTileSize
     * @param baseTileSize Base tile size (screenHeight / 12)
     * @param sceneWidth Width of the scene for positioning calculations
     */
    virtual void updateGeometry(float baseTileSize, float sceneWidth);

    /**
     * Get corner radius for rounded tile appearance
     * @param baseTileSize Base tile size
     * @return Corner radius (baseTileSize / 8)
     */
    static float cornerRadius(float baseTileSize);

    /**
     * Get current tile state
     */
    TileState state() const { return _state; }

    /**
     * Set tile state (changes appearance)
     */
    void setState(TileState state);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    float _widthMultiplier;
    float _heightMultiplier;
    Anchor _anchor;
    int _gridX;
    int _gridY;
    TileState _state;
    float _currentBaseTileSize;  // Accessible to derived classes for rendering
    float _width;
    float _height;
};

#endif // TILE_H
