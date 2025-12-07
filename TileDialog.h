#ifndef TILEDIALOG_H
#define TILEDIALOG_H

#include "Tile.h"
#include <QGraphicsRectItem>

/**
 * Modal dialog tile with semi-transparent overlay
 * Centered horizontally, positioned vertically via grid Y coordinate
 * Displayed with dimmed background overlay
 */
class TileDialog : public Tile
{
    Q_OBJECT

public:
    TileDialog(float widthMultiplier = 4.0f, float heightMultiplier = 6.0f,
               int gridY = 0, QGraphicsItem* parent = nullptr);
    
    virtual ~TileDialog();

    void show(float baseTileSize, float sceneWidth);
    void hide();
    bool isVisible() const;
    
    /**
     * @brief Update geometry of dialog and child tiles
     */
    void updateGeometry(float baseTileSize, float sceneWidth) override;

signals:
    void accepted();
    void rejected();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    QGraphicsRectItem* _dimOverlay;
};

#endif // TILEDIALOG_H
