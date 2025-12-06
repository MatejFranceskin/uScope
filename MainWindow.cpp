#include "MainWindow.h"
#include "VideoGraphicsScene.h"
#include "Tile.h"
#include "TileButton.h"
#include "controllers/CameraController.h"
#include "ui/CameraControlsPanel.h"
#include "models/CapturedImage.h"
#include <QGraphicsView>
#include <QResizeEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _view(nullptr)
    , _scene(nullptr)
    , _cameraController(nullptr)
    , _cameraPanel(nullptr)
{
    setupUi();
    createControllers();
    createTiles();
    
    // Auto-start first available camera
    _cameraController->autoStartCamera();
}

MainWindow::~MainWindow()
{
    // Qt parent-child ownership handles cleanup
}

void MainWindow::setupUi()
{
    // Create graphics scene with video background
    _scene = new VideoGraphicsScene(this);
    
    // Create graphics view
    _view = new QGraphicsView(_scene, this);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setRenderHint(QPainter::Antialiasing, true);
    _view->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    setCentralWidget(_view);
    
    // Set initial window size
    resize(1280, 720);
    setWindowTitle("uScope - Microscopy Platform");
    
    // Set window icon (prefer .ico on Windows/Linux, use .svg as fallback for Android/iOS)
    QIcon appIcon(":/images/uScope.ico");
    if (appIcon.isNull()) {
        appIcon = QIcon(":/images/uScope.svg");
    }
    setWindowIcon(appIcon);
}

void MainWindow::createControllers()
{
    // Create camera controller
    _cameraController = new CameraController(this);
    
    // Connect camera signals
    connect(_cameraController, &CameraController::frameReady,
            _scene, &VideoGraphicsScene::updateVideoFrame);
    connect(_cameraController, &CameraController::imageCaptured,
            this, &MainWindow::onImageCaptured);
    connect(_cameraController, &CameraController::error,
            this, &MainWindow::onCameraError);
}

void MainWindow::createTiles()
{
    // Create camera controls panel (hidden initially)
    _cameraPanel = new CameraControlsPanel(_cameraController);
    _scene->addItem(_cameraPanel);
    _cameraPanel->hide();
    
    connect(_cameraPanel, &CameraControlsPanel::captureRequested,
            this, &MainWindow::onCaptureButtonClicked);
    
    // Create left-anchored tiles
    TileButton* cameraButton = new TileButton(":/images/camera.svg", "Camera", 1.0f, 1.0f, Tile::Anchor::Left);
    _scene->addItem(cameraButton);
    _leftTiles.append(cameraButton);
    connect(cameraButton, &TileButton::clicked, this, &MainWindow::onCameraButtonClicked);
    
    TileButton* settingsButton = new TileButton(":/images/settings.svg", "Settings", 1.0f, 1.0f, Tile::Anchor::Left);
    _scene->addItem(settingsButton);
    _leftTiles.append(settingsButton);
    
    // Create right-anchored tiles
    TileButton* captureButton = new TileButton(":/images/media-record.svg", "Capture", 1.0f, 1.0f, Tile::Anchor::Right);
    _scene->addItem(captureButton);
    _rightTiles.append(captureButton);
    connect(captureButton, &TileButton::clicked, this, &MainWindow::onCaptureButtonClicked);
    
    TileButton* recordButton = new TileButton(":/images/media-playback-start.svg", "Record", 1.0f, 1.0f, Tile::Anchor::Right);
    _scene->addItem(recordButton);
    _rightTiles.append(recordButton);
}

void MainWindow::onCameraButtonClicked()
{
    if (_cameraPanel->isVisible()) {
        _cameraPanel->hide();
    } else {
        _cameraPanel->refreshCameras();
        _cameraPanel->show();
    }
}

void MainWindow::onCaptureButtonClicked()
{
    _cameraController->captureImage();
}

void MainWindow::onImageCaptured(const CapturedImage& image)
{
    QMessageBox::information(this, "Image Captured", 
        QString("Image saved to:\n%1").arg(image.filePath()));
}

void MainWindow::onCameraError(const QString& message)
{
    QMessageBox::warning(this, "Camera Error", message);
}

float MainWindow::calculateBaseTileSize() const
{
    return static_cast<float>(height()) / 12.0f;
}

void MainWindow::updateTileLayout()
{
    float baseTileSize = calculateBaseTileSize();
    QRectF sceneRect = _scene->sceneRect();
    
    // Update left-anchored tiles
    for (int i = 0; i < _leftTiles.size(); ++i) {
        Tile* tile = _leftTiles[i];
        tile->updateGeometry(baseTileSize, i);
        
        float yPos = tile->property("yPosition").toFloat();
        tile->setPos(baseTileSize * 0.5f, yPos);  // 0.5 tile spacing from left edge
    }
    
    // Update right-anchored tiles
    for (int i = 0; i < _rightTiles.size(); ++i) {
        Tile* tile = _rightTiles[i];
        tile->updateGeometry(baseTileSize, i);
        
        float yPos = tile->property("yPosition").toFloat();
        float xPos = sceneRect.width() - tile->boundingRect().width() - (baseTileSize * 0.5f);
        tile->setPos(xPos, yPos);
    }
    
    // Update center-anchored tiles
    for (int i = 0; i < _centerTiles.size(); ++i) {
        Tile* tile = _centerTiles[i];
        tile->updateGeometry(baseTileSize, i);
        
        float yPos = tile->property("yPosition").toFloat();
        float xPos = (sceneRect.width() - tile->boundingRect().width()) / 2.0f;
        tile->setPos(xPos, yPos);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // Update scene rect to match view
    if (_scene && _view) {
        _scene->setSceneRect(0, 0, _view->viewport()->width(), _view->viewport()->height());
    }
    
    updateTileLayout();
}
