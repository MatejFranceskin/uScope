#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QList>

class VideoGraphicsScene;
class Tile;
class TileButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void createPlaceholderTiles();
    float calculateBaseTileSize() const;
    void updateTileLayout();

    QGraphicsView *_view;
    VideoGraphicsScene *_scene;
    QList<Tile*> _leftTiles;
    QList<Tile*> _rightTiles;
    QList<Tile*> _centerTiles;
};

#endif // MAINWINDOW_H
