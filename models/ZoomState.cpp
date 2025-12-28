#include "ZoomState.h"
#include <QtMath>
#include <QDebug>

ZoomState::ZoomState(QObject* parent)
    : QObject(parent)
    , _zoomFactor(1.0)
    , _mode(Mode::Custom)  // Start in Custom mode, will calculate initial fit
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

void ZoomState::setZoomFactorInternal(qreal factor)
{
    // Set zoom factor without changing mode (for internal use by mode setters)
    qreal clampedFactor = clampZoomFactor(factor);
    qDebug() << "setZoomFactorInternal: factor=" << factor << "clamped=" << clampedFactor << "current=" << _zoomFactor << "mode=" << _mode;
    if (!qFuzzyCompare(_zoomFactor, clampedFactor)) {
        _zoomFactor = clampedFactor;
        emit zoomFactorChanged(_zoomFactor);
        emit stateChanged();
    }
}

void ZoomState::setMode(Mode mode)
{
    if (_mode != mode) {
        _mode = mode;
        emit modeChanged(_mode);
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
    setMode(Mode::Custom);
    setZoomFactor(1.0);
    setPanOffset(QPointF(0, 0));
}

QTransform ZoomState::calculateViewTransform(const QSizeF& viewportSize, const QSizeF& contentSize) const
{
    if (contentSize.isEmpty() || viewportSize.isEmpty()) {
        return QTransform();
    }

    QTransform transform;
    
    // Calculate scaled content size
    qreal scale = _zoomFactor;
    qreal scaledWidth = contentSize.width() * scale;
    qreal scaledHeight = contentSize.height() * scale;
    
    // Calculate centering offset
    qreal xCenter = (viewportSize.width() - scaledWidth) / 2.0;
    qreal yCenter = (viewportSize.height() - scaledHeight) / 2.0;
    
    // Build transform: scale first, then translate for centering and pan
    // Pan is in content coordinates, so apply it before scaling
    transform.scale(scale, scale);
    transform.translate(_panOffset.x(), _panOffset.y());
    transform.translate(xCenter / scale, yCenter / scale);

    return transform;
}

QString ZoomState::displayString() const
{
    // Always show the actual zoom factor value
    return QString("%1x").arg(_zoomFactor, 0, 'f', 1);
}

qreal ZoomState::clampZoomFactor(qreal factor) const
{
    return qBound(MIN_ZOOM, factor, MAX_ZOOM);
}
