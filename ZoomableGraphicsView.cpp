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
        // Get mouse position in scene coordinates
        QPointF mousePos = mapToScene(event->position().toPoint());
        
        // Get current zoom before change
        qreal oldZoom = _zoomController->zoomState()->zoomFactor();
        qreal newZoom = oldZoom;
        
        if (delta > 0) {
            // Zoom in
            newZoom = oldZoom + 0.1;
        } else {
            // Zoom out
            newZoom = oldZoom - 0.1;
        }
        
        // Clamp to valid range (will be done by ZoomController, but we need it for calculation)
        newZoom = qBound(0.1, newZoom, 10.0);
        
        qDebug() << "wheelEvent: mousePos=" << mousePos << "oldZoom=" << oldZoom << "newZoom=" << newZoom;
        
        // The mouse position is in scene coordinates, but we need to account for the
        // centering offset that's baked into the transform. 
        // The pan offset in our transform is applied AFTER centering and scaling.
        
        QPointF currentPan = _zoomController->zoomState()->panOffset();
        qreal zoomRatio = newZoom / oldZoom;
        
        qDebug() << "wheelEvent: currentPan=" << currentPan << "zoomRatio=" << zoomRatio;
        
        // The zoom-to-point formula: we want to keep the point under the mouse fixed
        // newPan = (mousePos - viewportCenter) / oldZoom - (mousePos - viewportCenter) / newZoom
        // Simplified: newPan = currentPan + mousePos * (1/newZoom - 1/oldZoom)
        // Or: newPan = currentPan * (oldZoom/newZoom) + mousePos * (1 - oldZoom/newZoom)
        
        // Account for viewport center
        QSizeF viewportSize(_zoomController->contentSize());
        QPointF viewportCenter(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF mouseOffset = mousePos - viewportCenter;
        
        // Adjust pan to keep mouse position fixed
        QPointF newPan = currentPan + mouseOffset * (1.0 / newZoom - 1.0 / oldZoom) * oldZoom;
        
        qDebug() << "wheelEvent: newPan=" << newPan << "mouseOffset=" << mouseOffset;
        
        // Apply zoom and pan together to avoid double transform
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
        
        // Pan by delta (negate to make drag feel natural - drag right = pan right)
        _zoomController->panBy(QPointF(-delta.x(), -delta.y()));
        
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
