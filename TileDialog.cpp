#include "TileDialog.h"
#include <QGraphicsScene>
#include <QKeyEvent>

TileDialog::TileDialog(float widthMultiplier, float heightMultiplier, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, Anchor::Center, parent)
    , _dimOverlay(nullptr)
{
    // Dialogs start hidden
    QGraphicsWidget::setVisible(false);
}

TileDialog::~TileDialog()
{
    if (_dimOverlay) {
        delete _dimOverlay;
    }
}

void TileDialog::show()
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
    
    // Center dialog in scene
    QPointF center = sceneRect.center();
    setPos(center.x() - boundingRect().width() / 2, 
           center.y() - boundingRect().height() / 2);
    
    // Bring dialog to front
    setZValue(1000);
    
    // Show dialog
    QGraphicsWidget::setVisible(true);
    setFocus();
}

void TileDialog::hide()
{
    QGraphicsWidget::setVisible(false);
    
    if (_dimOverlay) {
        _dimOverlay->setVisible(false);
    }
}

bool TileDialog::isVisible() const
{
    return QGraphicsWidget::isVisible();
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
