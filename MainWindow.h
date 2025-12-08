#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

class VideoGraphicsScene;
class ZoomableGraphicsView;
class Tile;
class TileButton;
class ZoomTile;
class CameraController;
class ZoomController;
class CameraControlsPanel;
class CapturedImage;
class QSoundEffect;
class QGraphicsRectItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void onFullscreenToggleClicked();
    void onCameraButtonClicked();
    void onCaptureButtonClicked();
    void onRecordButtonClicked();
    void onSettingsButtonClicked();
    void onImageCaptured(const CapturedImage& image);
    void onCameraError(const QString& message);
    void onCameraDisconnected(const QString& cameraId);
    void onZoomChanged(qreal factor, int mode);

private:
    void setupUi();
    void createControllers();
    void createTiles();
    float calculateBaseTileSize() const;
    int calculateFontSize() const;  // Centralized font size calculation
    void updateTileLayout();
    void setFullscreenMode(bool enabled);
    void showCaptureFlash();  // Visual feedback for image capture

    ZoomableGraphicsView *_view;
    VideoGraphicsScene *_scene;
    CameraController *_cameraController;
    ZoomController *_zoomController;
    CameraControlsPanel *_cameraPanel;
    ZoomTile *_zoomTile;
    
    QSoundEffect *_shutterSound;
    QGraphicsRectItem *_flashOverlay;
    
    TileButton* _fullscreenToggle;
    TileButton* _recordButton;
    bool _fullscreenMode;
    bool _isRecording;
    
    QList<Tile*> _leftTiles;
    QList<Tile*> _rightTiles;
    QList<Tile*> _centerTiles;
};

#endif // MAINWINDOW_H
