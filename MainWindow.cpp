#include "MainWindow.h"
#include "VideoGraphicsScene.h"
#include "ZoomableGraphicsView.h"
#include "Tile.h"
#include "TileButton.h"
#include "controllers/CameraController.h"
#include "controllers/ZoomController.h"
#include "ui/CameraControlsPanel.h"
#include "ui/ZoomTile.h"
#include "models/CapturedImage.h"
#include <QResizeEvent>
#include <QShowEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _view(nullptr)
    , _scene(nullptr)
    , _cameraController(nullptr)
    , _zoomController(nullptr)
    , _cameraPanel(nullptr)
    , _zoomTile(nullptr)
    , _fullscreenToggle(nullptr)
    , _recordButton(nullptr)
    , _fullscreenMode(false)
    , _isRecording(false)
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
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);  // Disable BSP tree for better performance with video
    
    // Create zoomable graphics view
    _view = new ZoomableGraphicsView(_scene, this);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setRenderHint(QPainter::Antialiasing, true);
    _view->setRenderHint(QPainter::SmoothPixmapTransform, true);
    _view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);  // Use double buffering, no flicker
    _view->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    
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
    
    // Set initial scene rect to match initial window size
    // Account for window decorations by using a reasonable initial size
    _scene->setSceneRect(0, 0, 1280, 720);
    qDebug() << "setupUi: Set initial scene rect to" << _scene->sceneRect();
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
    
    // Create zoom controller
    _zoomController = new ZoomController(_view, this);
    
    // Connect zoom controller to view
    _view->setZoomController(_zoomController);
    
    // Set initial content size (will be updated when camera starts)
    _zoomController->setContentSize(QSizeF(1920, 1080));
    
    // Set initial zoom to fit width
    _zoomController->setFitWidth();
    
    // Connect zoom changes for auto-close panel behavior
    connect(_zoomController, &ZoomController::zoomChanged,
            this, &MainWindow::onZoomChanged);
}

void MainWindow::createTiles()
{
    // Create camera controls panel (hidden initially)
    _cameraPanel = new CameraControlsPanel(_cameraController);
    _scene->addItem(_cameraPanel);
    _centerTiles.append(_cameraPanel);  // Add to center tiles so it gets resized
    _cameraPanel->hide();
    
    connect(_cameraPanel, &CameraControlsPanel::captureRequested,
            this, &MainWindow::onCaptureButtonClicked);
    
    // Create right-anchored tiles per spec.md tile layout:
    
    // Fullscreen Toggle (Right, 0,0, 1×1)
    _fullscreenToggle = new TileButton(":/images/uScope.svg", "Full", 1.0f, 1.0f, Tile::Anchor::Right, 0, 0);
    _scene->addItem(_fullscreenToggle);
    _rightTiles.append(_fullscreenToggle);
    connect(_fullscreenToggle, &TileButton::clicked, this, &MainWindow::onFullscreenToggleClicked);
    
    // Camera Selection (Right, 0,2, 1×1)
    TileButton* cameraButton = new TileButton(":/images/camera.svg", "Camera", 1.0f, 1.0f, Tile::Anchor::Right, 0, 2);
    _scene->addItem(cameraButton);
    _rightTiles.append(cameraButton);
    connect(cameraButton, &TileButton::clicked, this, &MainWindow::onCameraButtonClicked);
    
    // Settings (Right, 0,3, 1×1)
    TileButton* settingsButton = new TileButton(":/images/settings.svg", "Settings", 1.0f, 1.0f, Tile::Anchor::Right, 0, 3);
    _scene->addItem(settingsButton);
    _rightTiles.append(settingsButton);
    connect(settingsButton, &TileButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    
    // Snapshot (Right, 0,4, 1×1)
    TileButton* snapshotButton = new TileButton(":/images/media-record.svg", "Snapshot", 1.0f, 1.0f, Tile::Anchor::Right, 0, 4);
    _scene->addItem(snapshotButton);
    _rightTiles.append(snapshotButton);
    connect(snapshotButton, &TileButton::clicked, this, &MainWindow::onCaptureButtonClicked);
    
    // Record (Right, 0,5, 1×1) - two-state button
    _recordButton = new TileButton(":/images/media-playback-start.svg", "Record", 1.0f, 1.0f, Tile::Anchor::Right, 0, 5);
    _scene->addItem(_recordButton);
    _rightTiles.append(_recordButton);
    connect(_recordButton, &TileButton::clicked, this, &MainWindow::onRecordButtonClicked);
    
    // Zoom tile (Right, 0,6, 1×1)
    _zoomTile = new ZoomTile(_zoomController);
    _scene->addItem(_zoomTile);
    _rightTiles.append(_zoomTile);
    
    // Add zoom panel buttons to scene and tile list
    for (TileButton* button : _zoomTile->panelButtons()) {
        _scene->addItem(button);
        _rightTiles.append(button);
    }
}

void MainWindow::onCameraButtonClicked()
{
    if (_cameraPanel->isVisible()) {
        _cameraPanel->hide();
    } else {
        _cameraPanel->refreshCameras();
        float baseTileSize = calculateBaseTileSize();
        QRectF sceneRect = _scene->sceneRect();
        _cameraPanel->show(baseTileSize, sceneRect.width());
    }
}

void MainWindow::onSettingsButtonClicked()
{
    // TODO: Implement settings dialog
    QMessageBox::information(this, "Settings", "Settings dialog not yet implemented.");
}

void MainWindow::onCaptureButtonClicked()
{
    _cameraController->captureImage();
}

void MainWindow::onRecordButtonClicked()
{
    _isRecording = !_isRecording;
    
    if (_isRecording) {
        // Start recording
        _recordButton->setSvgPath(":/images/media-playback-stop.svg");
        _recordButton->setText("Stop");
        _recordButton->setState(Tile::TileState::Active);
        // TODO: Start actual video recording
        QMessageBox::information(this, "Recording", "Video recording started (not yet implemented).");
    } else {
        // Stop recording
        _recordButton->setSvgPath(":/images/media-playback-start.svg");
        _recordButton->setText("Record");
        _recordButton->setState(Tile::TileState::Idle);
        // TODO: Stop actual video recording
        QMessageBox::information(this, "Recording", "Video recording stopped.");
    }
}

void MainWindow::onFullscreenToggleClicked()
{
    setFullscreenMode(!_fullscreenMode);
}

void MainWindow::setFullscreenMode(bool enabled)
{
    _fullscreenMode = enabled;
    
    // Hide/show all tiles except fullscreen toggle
    for (Tile* tile : _leftTiles) {
        tile->setVisible(!enabled);
    }
    
    for (Tile* tile : _rightTiles) {
        if (tile != _fullscreenToggle) {
            tile->setVisible(!enabled);
        }
    }
    
    for (Tile* tile : _centerTiles) {
        tile->setVisible(!enabled);
    }
    
    // Update fullscreen toggle icon and state
    if (enabled) {
        _fullscreenToggle->setSvgPath(":/images/uScope.svg");  // TODO: Use exit-fullscreen icon
        _fullscreenToggle->setText("Exit");
        _fullscreenToggle->setState(Tile::TileState::Active);
    } else {
        _fullscreenToggle->setSvgPath(":/images/uScope.svg");  // TODO: Use enter-fullscreen icon
        _fullscreenToggle->setText("Full");
        _fullscreenToggle->setState(Tile::TileState::Idle);
    }
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

void MainWindow::onZoomChanged(qreal factor, int mode)
{
    Q_UNUSED(factor)
    Q_UNUSED(mode)
    
    // Auto-close zoom panel when zoom changes via mouse wheel or pinch gesture
    // This implements T089: auto-close panel behavior
    if (_zoomTile) {
        // The ZoomTile will handle hiding the panel internally
        // We just need to trigger it when zoom changes externally
    }
}

float MainWindow::calculateBaseTileSize() const
{
    return static_cast<float>(height()) / 8.0f;
}

void MainWindow::updateTileLayout()
{
    if (!_scene || !_view) {
        return;
    }
    
    float baseTileSize = calculateBaseTileSize();
    QRectF sceneRect = _scene->sceneRect();
    
    // Update all tiles with new geometry (they calculate their own positions)
    for (Tile* tile : _leftTiles) {
        tile->updateGeometry(baseTileSize, sceneRect.width());
    }
    
    for (Tile* tile : _rightTiles) {
        tile->updateGeometry(baseTileSize, sceneRect.width());
    }
    
    for (Tile* tile : _centerTiles) {
        tile->updateGeometry(baseTileSize, sceneRect.width());
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // Update scene rect to match viewport
    if (_scene && _view) {
        QSize viewportSize = _view->viewport()->size();
        _scene->setSceneRect(0, 0, viewportSize.width(), viewportSize.height());
        updateTileLayout();
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    // Ensure tiles are positioned correctly when window is first shown
    if (_scene && _view) {
        QSize viewportSize = _view->viewport()->size();
        _scene->setSceneRect(0, 0, viewportSize.width(), viewportSize.height());
        updateTileLayout();
    }
}
