#include "VideoGraphicsScene.h"
#include <QPainter>

VideoGraphicsScene::VideoGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
{
}

VideoGraphicsScene::~VideoGraphicsScene()
{
}

void VideoGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Draw black background as placeholder for video feed
    painter->fillRect(rect, Qt::black);
}
