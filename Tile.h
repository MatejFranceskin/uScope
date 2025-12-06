#ifndef TILE_H
#define TILE_H

#include <QGraphicsWidget>
#include <QPainter>

/**
 * Base class for tile-based UI overlay components
 * 
 * Tiles use proportional sizing based on baseTileSize = screenHeight / 12
 * and support anchoring to Left, Right, or Center screen positions.
 */
class Tile : public QGraphicsWidget
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
     * Construct a tile with size multipliers and anchor position
     * @param widthMultiplier Multiplier for baseTileSize to determine width
     * @param heightMultiplier Multiplier for baseTileSize to determine height
     * @param anchor Screen position anchor (Left, Right, Center)
     * @param parent Parent graphics item
     */
    Tile(float widthMultiplier, float heightMultiplier, Anchor anchor, QGraphicsItem* parent = nullptr);

    /**
     * Update tile geometry based on baseTileSize and position index
     * @param baseTileSize Base tile size (screenHeight / 12)
     * @param positionIndex Index of tile in anchor list (0 = top)
     */
    void updateGeometry(float baseTileSize, int positionIndex);

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
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    float _widthMultiplier;
    float _heightMultiplier;
    Anchor _anchor;
    TileState _state;

private:
    float _currentBaseTileSize;
};

#endif // TILE_H
