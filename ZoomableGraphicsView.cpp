#include "ZoomableGraphicsView.h"
#include "controllers/ZoomController.h"
#include "models/ZoomState.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QScrollBar>

ZoomableGraphicsView::ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _zoomController(nullptr)
    , _isPanning(false)
    , _lastPanPos(0, 0)
    , _lastPinchScale(1.0)
{
    // Enable mouse tracking for cursor changes
    setMouseTracking(true);
    
    // Enable pinch gesture
    grabGesture(Qt::PinchGesture);
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

bool ZoomableGraphicsView::event(QEvent* event)
{
    if (!_zoomController) {
        return QGraphicsView::event(event);
    }
    
    // Handle gesture events
    if (event->type() == QEvent::Gesture) {
        QGestureEvent* gestureEvent = static_cast<QGestureEvent*>(event);
        
        if (QGesture* gesture = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture* pinch = static_cast<QPinchGesture*>(gesture);
            
            if (pinch->state() == Qt::GestureStarted) {
                // Initialize pinch scale
                _lastPinchScale = 1.0;
            } else if (pinch->state() == Qt::GestureUpdated) {
                // Calculate scale factor change
                qreal currentScale = pinch->totalScaleFactor();
                qreal scaleDelta = currentScale / _lastPinchScale;
                _lastPinchScale = currentScale;
                
                // Apply zoom based on scale delta
                qreal currentZoom = _zoomController->zoomState()->zoomFactor();
                qreal newZoom = currentZoom * scaleDelta;
                _zoomController->setZoomFactor(newZoom);
            } else if (pinch->state() == Qt::GestureFinished || 
                       pinch->state() == Qt::GestureCanceled) {
                // Reset pinch scale
                _lastPinchScale = 1.0;
            }
            
            event->accept();
            return true;
        }
    }
    
    // Pass to base class
    return QGraphicsView::event(event);
}
