#include "ZoomableGraphicsView.h"
#include "controllers/ZoomController.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

ZoomableGraphicsView::ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _zoomController(nullptr)
    , _isPanning(false)
    , _lastPanPos(0, 0)
{
    // Enable mouse tracking for cursor changes
    setMouseTracking(true);
}

void ZoomableGraphicsView::setZoomController(ZoomController* controller)
{
    _zoomController = controller;
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (!_zoomController) {
        QGraphicsView::wheelEvent(event);
        return;
    }
    
    // Get wheel delta (positive = zoom in, negative = zoom out)
    int delta = event->angleDelta().y();
    
    if (delta > 0) {
        // Zoom in
        _zoomController->zoomIn();
    } else if (delta < 0) {
        // Zoom out
        _zoomController->zoomOut();
    }
    
    // Accept event to prevent propagation
    event->accept();
}

void ZoomableGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (!_zoomController) {
        QGraphicsView::mousePressEvent(event);
        return;
    }
    
    // Check if middle button or Ctrl+left button for panning
    if (event->button() == Qt::MiddleButton || 
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier)) {
        
        _isPanning = true;
        _lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    // Pass to base class for normal item interaction
    QGraphicsView::mousePressEvent(event);
}

void ZoomableGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (!_zoomController) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    
    if (_isPanning) {
        // Calculate delta in scene coordinates
        QPoint delta = event->pos() - _lastPanPos;
        _lastPanPos = event->pos();
        
        // Pan by delta
        _zoomController->panBy(QPointF(delta.x(), delta.y()));
        
        event->accept();
        return;
    }
    
    // Pass to base class
    QGraphicsView::mouseMoveEvent(event);
}

void ZoomableGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!_zoomController) {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    
    if (_isPanning && (event->button() == Qt::MiddleButton || 
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier))) {
        
        _isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    
    // Pass to base class
    QGraphicsView::mouseReleaseEvent(event);
}
