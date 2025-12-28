#include "ZoomController.h"
#include "../models/ZoomState.h"
#include "../VideoGraphicsScene.h"
#include <QGraphicsView>
#include <QDebug>

ZoomController::ZoomController(QGraphicsView* view, QObject* parent)
    : QObject(parent)
    , _zoomState(new ZoomState(this))
    , _view(view)
    , _scene(nullptr)
    , _contentSize(1920, 1080)  // Default content size
    , _updatingTransform(false)
{
    // Get the scene from the view
    if (view) {
        _scene = qobject_cast<VideoGraphicsScene*>(view->scene());
    }
    
    // Connect ZoomState signals to controller slots
    connect(_zoomState, &ZoomState::stateChanged,
            this, &ZoomController::onZoomStateChanged);
    connect(_zoomState, &ZoomState::zoomFactorChanged,
            this, [this](qreal factor) {
                emit zoomChanged(factor, static_cast<int>(_zoomState->mode()));
            });
    connect(_zoomState, &ZoomState::panOffsetChanged,
            this, &ZoomController::panChanged);
    
    // Apply initial transform
    applyTransform();
}

ZoomController::~ZoomController()
{
    // ZoomState deleted automatically via parent ownership
}

void ZoomController::setContentSize(const QSizeF& size)
{
    if (_contentSize != size) {
        _contentSize = size;
        
        // When content size changes, recalculate zoom for fit modes
        // This ensures that fit-width/fit-height scale is updated for new content
        switch (_zoomState->mode()) {
            case ZoomState::Mode::FitWidth:
                setFitWidth();
                break;
            case ZoomState::Mode::FitHeight:
                setFitHeight();
                break;
            case ZoomState::Mode::OneToOne:
                // For 1:1, just recalculate transform with zoom factor 1.0
                applyTransform();
                break;
            case ZoomState::Mode::Custom:
                // For custom zoom, keep current zoom factor
                applyTransform();
                break;
        }
    }
}

void ZoomController::setZoomFactor(qreal factor)
{
    _zoomState->setZoomFactor(factor);
}

void ZoomController::zoomIn()
{
    qreal newFactor = _zoomState->zoomFactor() + 0.1;
    _zoomState->setZoomFactor(newFactor);
}

void ZoomController::zoomOut()
{
    qreal newFactor = _zoomState->zoomFactor() - 0.1;
    _zoomState->setZoomFactor(newFactor);
}

void ZoomController::setFitWidth()
{
    // Calculate zoom factor for fit width
    QSizeF viewportSize = _scene->sceneRect().size();
    qreal scale = viewportSize.width() / _contentSize.width();
    
    qDebug() << "setFitWidth: viewportSize=" << viewportSize << "contentSize=" << _contentSize << "scale=" << scale;
    
    // Set mode first, then zoom factor without changing mode
    _zoomState->setMode(ZoomState::Mode::FitWidth);
    _zoomState->setZoomFactorInternal(scale);
    
    // Reset pan to center
    _zoomState->setPanOffset(QPointF(0, 0));
}

void ZoomController::setFitHeight()
{
    // Calculate zoom factor for fit height
    QSizeF viewportSize = _scene->sceneRect().size();
    qreal scale = viewportSize.height() / _contentSize.height();
    
    // Set mode first, then zoom factor without changing mode
    _zoomState->setMode(ZoomState::Mode::FitHeight);
    _zoomState->setZoomFactorInternal(scale);
    
    // Reset pan to center
    _zoomState->setPanOffset(QPointF(0, 0));
}

void ZoomController::setOneToOne()
{
    // Set mode first, then zoom factor to 1.0
    _zoomState->setMode(ZoomState::Mode::OneToOne);
    _zoomState->setZoomFactorInternal(1.0);
    
    // Reset pan to center
    _zoomState->setPanOffset(QPointF(0, 0));
}

void ZoomController::setPanOffset(const QPointF& offset)
{
    _zoomState->setPanOffset(offset);
}

void ZoomController::setZoomAndPan(qreal factor, const QPointF& offset)
{
    // Set both zoom and pan atomically to avoid double transform application
    _zoomState->setZoomFactor(factor);
    _zoomState->setPanOffset(offset);
}

void ZoomController::panBy(const QPointF& delta)
{
    QPointF newOffset = _zoomState->panOffset() + delta;
    _zoomState->setPanOffset(newOffset);
}

void ZoomController::reset()
{
    _zoomState->reset();
}

void ZoomController::updateViewportSize()
{
    // Reapply the current mode to recalculate for new viewport size
    // IMPORTANT: Save and restore pan offset to avoid losing user's pan position
    QPointF savedPan = _zoomState->panOffset();
    
    switch (_zoomState->mode()) {
        case ZoomState::Mode::FitWidth:
            setFitWidth();
            break;
        case ZoomState::Mode::FitHeight:
            setFitHeight();
            break;
        case ZoomState::Mode::OneToOne:
            setOneToOne();
            break;
        case ZoomState::Mode::Custom:
            // For custom zoom, just recalculate transform with same zoom factor
            applyTransform();
            break;
    }
    
    // Restore user's pan offset
    _zoomState->setPanOffset(savedPan);
}

void ZoomController::onZoomStateChanged()
{
    // Apply transform whenever zoom state changes
    applyTransform();
    emit stateChanged();
}

void ZoomController::applyTransform()
{
    if (!_scene || _updatingTransform) {
        return;
    }
    
    _updatingTransform = true;

    // Get viewport size from scene rect (more reliable than viewport()->size())
    QSizeF viewportSize = _scene->sceneRect().size();
    
    // Calculate transform from ZoomState
    QTransform transform = _zoomState->calculateViewTransform(viewportSize, _contentSize);
    
    // Apply to video background layer only (not to tiles)
    _scene->setVideoTransform(transform);
    
    _updatingTransform = false;
}
