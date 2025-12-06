#ifndef TILEDIALOG_H
#define TILEDIALOG_H

#include "Tile.h"
#include <QGraphicsRectItem>

/**
 * Modal dialog tile with semi-transparent overlay
 * Displayed centered on screen with dimmed background
 */
class TileDialog : public Tile
{
    Q_OBJECT

public:
    TileDialog(float widthMultiplier = 4.0f, float heightMultiplier = 6.0f, 
               QGraphicsItem* parent = nullptr);
    
    virtual ~TileDialog();

    void show();
    void hide();
    bool isVisible() const;

signals:
    void accepted();
    void rejected();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QGraphicsRectItem* _dimOverlay;
};

#endif // TILEDIALOG_H
