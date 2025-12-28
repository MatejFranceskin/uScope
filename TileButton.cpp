#include "TileButton.h"
#include <QGraphicsSceneMouseEvent>

TileButton::TileButton(const QString& svgPath, const QString& text, float widthMultiplier, 
                       float heightMultiplier, Anchor anchor, int gridX, int gridY, QGraphicsItem* parent)
    : TileLabel(svgPath, text, widthMultiplier, heightMultiplier, anchor, gridX, gridY, parent)
    , _pressed(false)
{
}

void TileButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _state != TileState::Disabled) {
        _pressed = true;
        setState(TileState::Active);
        event->accept();
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void TileButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _pressed) {
        _pressed = false;
        
        // Check if release is still within button bounds
        if (boundingRect().contains(event->pos())) {
            // Return to Idle, hover events will set Hover if mouse is still over
            setState(TileState::Idle);
            emit clicked();
        } else {
            setState(TileState::Idle);
        }
        
        event->accept();
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}
