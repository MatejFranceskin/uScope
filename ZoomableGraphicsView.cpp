#include "ZoomableGraphicsView.h"
#include "controllers/ZoomController.h"
#include "models/ZoomState.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QScrollBar>
#include <QGraphicsItem>

ZoomableGraphicsView::ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , _zoomController(nullptr)
    , _isPanning(false)
    , _lastPanPos(0, 0)
    , _lastPinchScale(1.0)
{
    // Enable mouse tracking for cursor changes
    setMouseTracking(true);
    
    // Enable pinch gesture for touchpad zoom
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
    
    if (delta != 0) {
        // Get mouse position in viewport coordinates
        QPointF viewportPos = event->position();
        
        // Get current zoom and pan
        qreal oldZoom = _zoomController->zoomState()->zoomFactor();
        QPointF currentPan = _zoomController->zoomState()->panOffset();
        
        // Calculate new zoom
        qreal newZoom = oldZoom;
        if (delta > 0) {
            newZoom = oldZoom + 0.1;
        } else {
            newZoom = oldZoom - 0.1;
        }
        
        // Clamp to valid range
        newZoom = qBound(0.1, newZoom, 10.0);
        
        // Calculate viewport center
        QPointF viewportCenter(viewport()->width() / 2.0, viewport()->height() / 2.0);
        
        // Mouse offset from viewport center
        QPointF mouseOffset = viewportPos - viewportCenter;
        
        // To keep the point under the mouse fixed during zoom:
        // The point we want to keep fixed is at: mouseOffset from center
        // After centering, the content point under mouse is: (mouseOffset - currentPan * oldZoom) / oldZoom
        // We want this same content point to be under mouse after zoom:
        // (mouseOffset - newPan * newZoom) / newZoom = (mouseOffset - currentPan * oldZoom) / oldZoom
        // Solving for newPan:
        // mouseOffset - newPan * newZoom = (mouseOffset - currentPan * oldZoom) * (newZoom / oldZoom)
        // newPan * newZoom = mouseOffset - mouseOffset * (newZoom / oldZoom) + currentPan * oldZoom * (newZoom / oldZoom)
        // newPan = mouseOffset / newZoom - mouseOffset / oldZoom + currentPan * (oldZoom / newZoom) * (newZoom / newZoom)
        // newPan = mouseOffset * (1/newZoom - 1/oldZoom) + currentPan
        
        QPointF newPan = currentPan + mouseOffset * (1.0 / newZoom - 1.0 / oldZoom);
        
        // Apply zoom and pan together
        _zoomController->setZoomAndPan(newZoom, newPan);
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
    
    // For left button, check if clicking on an interactive item
    if (event->button() == Qt::LeftButton) {
        QGraphicsItem* item = itemAt(event->pos());
        
        // If clicking on an item that accepts mouse buttons, let it handle the event
        if (item) {
            // Check if item or any parent accepts mouse buttons
            while (item) {
                if (item->acceptedMouseButtons() != Qt::NoButton) {
                    QGraphicsView::mousePressEvent(event);
                    return;
                }
                item = item->parentItem();
            }
        }
        
        // No interactive item under mouse, start panning
        _isPanning = true;
        _lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    // Handle middle button panning
    if (event->button() == Qt::MiddleButton) {
        _isPanning = true;
        _lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    
    // Pass other events to base class
    QGraphicsView::mousePressEvent(event);
}

void ZoomableGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (!_zoomController) {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    
    if (_isPanning) {
        // Calculate delta in viewport coordinates
        QPoint delta = event->pos() - _lastPanPos;
        _lastPanPos = event->pos();
        
        // Convert delta to content coordinate space by dividing by zoom factor
        // Pan offset is applied in content coordinates, so we need to scale the viewport delta
        qreal zoom = _zoomController->zoomState()->zoomFactor();
        QPointF contentDelta(delta.x() / zoom, delta.y() / zoom);
        
        // Pan by delta
        _zoomController->panBy(QPointF(contentDelta.x(), contentDelta.y()));
        
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
    
    if (_isPanning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
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
        
        // Handle pinch gesture (two-finger zoom on touchpad)
        if (QGesture* gesture = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture* pinch = static_cast<QPinchGesture*>(gesture);
            
            if (pinch->state() == Qt::GestureStarted) {
                // Initialize pinch scale
                _lastPinchScale = 1.0;
            } else if (pinch->state() == Qt::GestureUpdated) {
                // Get pinch center point in scene coordinates
                QPointF centerPoint = mapToScene(pinch->centerPoint().toPoint());
                
                // Get current zoom before change
                qreal oldZoom = _zoomController->zoomState()->zoomFactor();
                
                // Calculate scale factor change
                qreal currentScale = pinch->totalScaleFactor();
                qreal scaleDelta = currentScale / _lastPinchScale;
                
                // Only apply if delta is significant (reduces jitter from touchpad)
                if (qAbs(scaleDelta - 1.0) < 0.01) {
                    event->accept();
                    return true;
                }
                
                _lastPinchScale = currentScale;
                
                // Apply zoom based on scale delta
                qreal newZoom = oldZoom * scaleDelta;
                
                // Calculate pan adjustment to zoom into center point
                // The center point should remain at the same position on screen
                QPointF currentPan = _zoomController->zoomState()->panOffset();
                qreal zoomRatio = newZoom / oldZoom;
                
                // Adjust pan so the center point stays fixed
                QPointF newPan = currentPan + (centerPoint - currentPan) * (1.0 - zoomRatio);
                
                _zoomController->setZoomAndPan(newZoom, newPan);
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

void ZoomableGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    
    // Update scene rect to match viewport size
    QRectF newSceneRect(0, 0, viewport()->width(), viewport()->height());
    scene()->setSceneRect(newSceneRect);
    
    // Recalculate zoom transform with new viewport size
    if (_zoomController) {
        _zoomController->updateViewportSize();
    }
}
