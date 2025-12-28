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
        
        // Apply the video transform which includes scale, centering, and pan
        painter->setTransform(_videoTransform, false);
        
        // Draw video at natural size from origin
        QSizeF frameSize = _currentFrame.size();
        QRectF targetRect(0, 0, frameSize.width(), frameSize.height());
        painter->drawImage(targetRect, _currentFrame);
        
        // Restore painter state
        painter->restore();
    }
}
