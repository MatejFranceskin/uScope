#ifndef VIDEOGRAPHICSSCENE_H
#define VIDEOGRAPHICSSCENE_H

#include <QGraphicsScene>

class VideoGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit VideoGraphicsScene(QObject *parent = nullptr);
    ~VideoGraphicsScene();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    // Background rendering will be implemented when camera integration is added
};

#endif // VIDEOGRAPHICSSCENE_H
