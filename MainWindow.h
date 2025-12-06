#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QList>

class VideoGraphicsScene;
class Tile;
class TileButton;
class CameraController;
class CameraControlsPanel;
class CapturedImage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCameraButtonClicked();
    void onCaptureButtonClicked();
    void onImageCaptured(const CapturedImage& image);
    void onCameraError(const QString& message);

private:
    void setupUi();
    void createControllers();
    void createTiles();
    float calculateBaseTileSize() const;
    void updateTileLayout();

    QGraphicsView *_view;
    VideoGraphicsScene *_scene;
    CameraController *_cameraController;
    CameraControlsPanel *_cameraPanel;
    
    QList<Tile*> _leftTiles;
    QList<Tile*> _rightTiles;
    QList<Tile*> _centerTiles;
};

#endif // MAINWINDOW_H
