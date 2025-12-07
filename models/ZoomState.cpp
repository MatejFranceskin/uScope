#include "ZoomState.h"
#include <QtMath>

ZoomState::ZoomState(QObject* parent)
    : QObject(parent)
    , _zoomFactor(1.0)
    , _mode(Mode::FitWidth)
    , _panOffset(0, 0)
{
}

void ZoomState::setZoomFactor(qreal factor)
{
    qreal clampedFactor = clampZoomFactor(factor);
    if (!qFuzzyCompare(_zoomFactor, clampedFactor)) {
        _zoomFactor = clampedFactor;
        _mode = Mode::Custom;
        emit zoomFactorChanged(_zoomFactor);
        emit modeChanged(_mode);
        emit stateChanged();
    }
}

void ZoomState::setMode(Mode mode)
{
    if (_mode != mode) {
        _mode = mode;
        
        // Set appropriate zoom factor for mode
        switch (mode) {
            case Mode::OneToOne:
                _zoomFactor = 1.0;
                break;
            case Mode::FitWidth:
            case Mode::FitHeight:
                // Zoom factor will be calculated dynamically based on viewport size
                _zoomFactor = 1.0;
                break;
            case Mode::Custom:
                // Keep current zoom factor
                break;
        }
        
        emit modeChanged(_mode);
        emit zoomFactorChanged(_zoomFactor);
        emit stateChanged();
    }
}

void ZoomState::setPanOffset(const QPointF& offset)
{
    if (_panOffset != offset) {
        _panOffset = offset;
        emit panOffsetChanged(_panOffset);
        emit stateChanged();
    }
}

void ZoomState::reset()
{
    setMode(Mode::FitWidth);
    setPanOffset(QPointF(0, 0));
}

QTransform ZoomState::calculateViewTransform(const QSizeF& viewportSize, const QSizeF& contentSize) const
{
    if (contentSize.isEmpty() || viewportSize.isEmpty()) {
        return QTransform();
    }

    QTransform transform;
    qreal scale = _zoomFactor;

    // Calculate scale based on mode
    switch (_mode) {
        case Mode::FitWidth:
            scale = viewportSize.width() / contentSize.width();
            break;
        
        case Mode::FitHeight:
            scale = viewportSize.height() / contentSize.height();
            break;
        
        case Mode::OneToOne:
            scale = 1.0;
            break;
        
        case Mode::Custom:
            scale = _zoomFactor;
            break;
    }

    // Apply scale
    transform.scale(scale, scale);

    // Apply pan offset
    transform.translate(_panOffset.x(), _panOffset.y());

    return transform;
}

QString ZoomState::displayString() const
{
    switch (_mode) {
        case Mode::FitWidth:
            return "Fit W";
        
        case Mode::FitHeight:
            return "Fit H";
        
        case Mode::OneToOne:
            return "100%";
        
        case Mode::Custom:
            return QString("%1x").arg(_zoomFactor, 0, 'f', 1);
    }
    
    return QString();
}

qreal ZoomState::clampZoomFactor(qreal factor) const
{
    return qBound(MIN_ZOOM, factor, MAX_ZOOM);
}
