#include "ZoomController.h"
#include "../models/ZoomState.h"
#include "../VideoGraphicsScene.h"
#include <QGraphicsView>

ZoomController::ZoomController(QGraphicsView* view, QObject* parent)
    : QObject(parent)
    , _zoomState(new ZoomState(this))
    , _view(view)
    , _scene(nullptr)
    , _contentSize(1920, 1080)  // Default content size
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
        // Recalculate transform with new content size
        applyTransform();
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
    _zoomState->setMode(ZoomState::Mode::FitWidth);
}

void ZoomController::setFitHeight()
{
    _zoomState->setMode(ZoomState::Mode::FitHeight);
}

void ZoomController::setOneToOne()
{
    _zoomState->setMode(ZoomState::Mode::OneToOne);
}

void ZoomController::setPanOffset(const QPointF& offset)
{
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

void ZoomController::onZoomStateChanged()
{
    // Apply transform whenever zoom state changes
    applyTransform();
    emit stateChanged();
}

void ZoomController::applyTransform()
{
    if (!_scene) {
        return;
    }

    // Get viewport size
    QSizeF viewportSize(_view->viewport()->width(), _view->viewport()->height());
    
    // Calculate transform from ZoomState
    QTransform transform = _zoomState->calculateViewTransform(viewportSize, _contentSize);
    
    // Apply to video background layer only (not to tiles)
    _scene->setVideoTransform(transform);
}
