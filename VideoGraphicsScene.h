#ifndef VIDEOGRAPHICSSCENE_H
#define VIDEOGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QVideoFrame>
#include <QImage>

class VideoGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit VideoGraphicsScene(QObject *parent = nullptr);
    ~VideoGraphicsScene();

public slots:
    void updateVideoFrame(const QVideoFrame& frame);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QImage _currentFrame;
};

#endif // VIDEOGRAPHICSSCENE_H
