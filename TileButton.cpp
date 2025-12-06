#include "TileButton.h"
#include <QGraphicsSceneMouseEvent>

TileButton::TileButton(const QString& svgPath, const QString& text, float widthMultiplier, 
                       float heightMultiplier, Anchor anchor, QGraphicsItem* parent)
    : TileLabel(svgPath, text, widthMultiplier, heightMultiplier, anchor, parent)
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
        QGraphicsWidget::mousePressEvent(event);
    }
}

void TileButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _pressed) {
        _pressed = false;
        
        // Check if release is still within button bounds
        if (boundingRect().contains(event->pos())) {
            setState(TileState::Hover);
            emit clicked();
        } else {
            setState(TileState::Idle);
        }
        
        event->accept();
    } else {
        QGraphicsWidget::mouseReleaseEvent(event);
    }
}
