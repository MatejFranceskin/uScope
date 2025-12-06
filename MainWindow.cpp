#include "MainWindow.h"
#include "VideoGraphicsScene.h"
#include <QGraphicsView>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _view(nullptr)
    , _scene(nullptr)
{
    setupUi();
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
}

int MainWindow::calculateBaseTileSize() const
{
    return height() / 12;
}

void MainWindow::updateTileLayout()
{
    int baseTileSize = calculateBaseTileSize();
    
    // Tile layout will be implemented in Phase 3
    // For now, just calculate the base size
    (void)baseTileSize; // Suppress unused variable warning
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
