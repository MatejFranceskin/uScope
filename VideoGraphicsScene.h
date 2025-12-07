#ifndef VIDEOGRAPHICSSCENE_H
#define VIDEOGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QVideoFrame>
#include <QImage>
#include <QTransform>

class VideoGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit VideoGraphicsScene(QObject *parent = nullptr);
    ~VideoGraphicsScene();

public slots:
    void updateVideoFrame(const QVideoFrame& frame);
    void setVideoTransform(const QTransform& transform);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QImage _currentFrame;
    QTransform _videoTransform;
};

#endif // VIDEOGRAPHICSSCENE_H
