#include "VideoGraphicsScene.h"
#include <QPainter>
#include <QGraphicsView>

VideoGraphicsScene::VideoGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , _videoTransform()
{
}

VideoGraphicsScene::~VideoGraphicsScene()
{
}

void VideoGraphicsScene::updateVideoFrame(const QVideoFrame& frame)
{
    if (frame.isValid()) {
        _currentFrame = frame.toImage();
        // Invalidate background layer only
        invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
    }
}

void VideoGraphicsScene::setVideoTransform(const QTransform& transform)
{
    _videoTransform = transform;
    // Invalidate background to redraw with new transform
    invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
}

void VideoGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, Qt::black);
    
    if (!_currentFrame.isNull()) {
        // Save painter state
        painter->save();
        
        // Apply video transform (zoom/pan)
        painter->setTransform(_videoTransform, true);
        
        // Scale video frame to fit scene while maintaining aspect ratio
        QRectF targetRect = rect;
        QSizeF frameSize = _currentFrame.size();
        QSizeF sceneSize = rect.size();
        
        qreal frameAspect = frameSize.width() / frameSize.height();
        qreal sceneAspect = sceneSize.width() / sceneSize.height();
        
        if (frameAspect > sceneAspect) {
            // Frame is wider - fit to width
            qreal scaledHeight = sceneSize.width() / frameAspect;
            qreal yOffset = (sceneSize.height() - scaledHeight) / 2.0;
            targetRect = QRectF(rect.left(), rect.top() + yOffset, sceneSize.width(), scaledHeight);
        } else {
            // Frame is taller - fit to height
            qreal scaledWidth = sceneSize.height() * frameAspect;
            qreal xOffset = (sceneSize.width() - scaledWidth) / 2.0;
            targetRect = QRectF(rect.left() + xOffset, rect.top(), scaledWidth, sceneSize.height());
        }
        
        painter->drawImage(targetRect, _currentFrame);
        
        // Restore painter state
        painter->restore();
    }
}
