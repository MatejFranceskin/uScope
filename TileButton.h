#ifndef TILEBUTTON_H
#define TILEBUTTON_H

#include "TileLabel.h"

/**
 * Interactive button tile with click signal and press animation
 */
class TileButton : public TileLabel
{
    Q_OBJECT

public:
    TileButton(const QString& svgPath, const QString& text, float widthMultiplier = 1.0f, 
               float heightMultiplier = 1.0f, Anchor anchor = Anchor::Center, 
               int gridX = 0, int gridY = 0, QGraphicsItem* parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    bool _pressed;
};

#endif // TILEBUTTON_H
