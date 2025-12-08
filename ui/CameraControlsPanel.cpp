#include "CameraControlsPanel.h"
#include "../controllers/CameraController.h"
#include "../controllers/ZoomController.h"
#include <QPainter>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <algorithm>

CameraControlsPanel::CameraControlsPanel(CameraController* cameraController,
                                         ZoomController* zoomController,
                                         QGraphicsItem* parent)
    : TileDialog(6.0f, 6.0f, 1, parent)  // 6×6 tiles - scroll bars show if content exceeds size
    , _controller(cameraController)
    , _zoomController(zoomController)
    , _cameraList(nullptr)
    , _formatList(nullptr)
    , _contentWidget(nullptr)
{
    setupUI();
}

void CameraControlsPanel::setupUI()
{
    // Create content widget with standard Qt controls
    QWidget* content = new QWidget();
    
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    
    // Camera selection label
    QLabel* cameraLabel = new QLabel("Select Camera:");
    cameraLabel->setFixedHeight(20);
    
    // Camera list widget
    _cameraList = new QListWidget();
    _cameraList->setStyleSheet(
        "QListWidget {"
        "   background-color: rgba(60, 60, 60, 200);"
        "   color: white;"
        "   border: 2px solid rgba(100, 100, 100, 200);"
        "   border-radius: 5px;"
        "   padding: 3px;"
        "}"
        "QListWidget::item {"
        "   padding: 5px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(80, 150, 80, 220);"
        "}"
        "QListWidget::item:hover {"
        "   background-color: rgba(120, 120, 150, 200);"
        "}"
    );
    connect(_cameraList, &QListWidget::currentRowChanged,
            this, &CameraControlsPanel::onCameraSelected);
    
    // Format selection label
    QLabel* formatLabel = new QLabel("Resolution & Frame Rate:");
    formatLabel->setFixedHeight(20);
    
    // Format list widget
    _formatList = new QListWidget();
    _formatList->setStyleSheet(
        "QListWidget {"
        "   background-color: rgba(60, 60, 60, 200);"
        "   color: white;"
        "   border: 2px solid rgba(100, 100, 100, 200);"
        "   border-radius: 5px;"
        "   padding: 3px;"
        "}"
        "QListWidget::item {"
        "   padding: 5px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(80, 150, 80, 220);"
        "}"
        "QListWidget::item:hover {"
        "   background-color: rgba(120, 120, 150, 200);"
        "}"
    );
    connect(_formatList, &QListWidget::currentRowChanged,
            this, &CameraControlsPanel::onFormatSelected);
    
    // Assemble layout
    layout->addWidget(cameraLabel);
    layout->addWidget(_cameraList);
    layout->addWidget(formatLabel);
    layout->addWidget(_formatList);
    
    // Store reference to content widget
    _contentWidget = content;
    
    // Don't set initial height - let updateGeometry handle it
    
    // Set as dialog content - scroll bars appear automatically if needed
    setContentWidget(content);
    
    // Initial camera refresh
    refreshCameras();
}

void CameraControlsPanel::refreshCameras()
{
    _availableCameras = _controller->availableCameras();
    
    _cameraList->clear();
    
    if (_availableCameras.isEmpty()) {
        _cameraList->addItem("No cameras detected");
    } else {
        for (const CameraProfile& camera : _availableCameras) {
            QListWidgetItem* item = new QListWidgetItem(camera.name());
            item->setData(Qt::UserRole, camera.id());
            _cameraList->addItem(item);
        }
        _cameraList->setCurrentRow(0);
    }
}

void CameraControlsPanel::onCameraSelected(int index)
{
    if (index >= 0 && index < _availableCameras.size()) {
        QString cameraId = _availableCameras[index].id();
        
        // Get available formats for this camera
        _availableFormats = _controller->availableFormats(cameraId);
        
        // Populate format list
        _formatList->clear();
        
        if (_availableFormats.isEmpty()) {
            _formatList->addItem("No formats available");
        } else {
            // Sort formats: higher resolution first, then higher frame rate
            std::sort(_availableFormats.begin(), _availableFormats.end(), 
                [](const QCameraFormat& a, const QCameraFormat& b) {
                    QSize resA = a.resolution();
                    QSize resB = b.resolution();
                    int pixelsA = resA.width() * resA.height();
                    int pixelsB = resB.width() * resB.height();
                    
                    if (pixelsA != pixelsB) {
                        return pixelsA > pixelsB;  // Higher resolution first
                    }
                    return a.maxFrameRate() > b.maxFrameRate();  // Higher fps first
                });
            
            // Get current format if camera is active
            QCameraFormat currentFormat = _controller->currentFormat();
            int selectedIndex = 0;
            
            for (int i = 0; i < _availableFormats.size(); ++i) {
                const QCameraFormat& format = _availableFormats[i];
                QSize resolution = format.resolution();
                qreal fps = format.maxFrameRate();
                
                QString formatText = QString("%1x%2 @ %3 fps")
                    .arg(resolution.width())
                    .arg(resolution.height())
                    .arg(fps, 0, 'f', 0);
                
                _formatList->addItem(formatText);
                
                // Check if this is the current format
                if (!currentFormat.isNull() && 
                    currentFormat.resolution() == resolution &&
                    qAbs(currentFormat.maxFrameRate() - fps) < 0.1) {
                    selectedIndex = i;
                }
            }
            
            // Select current format or first one
            _formatList->setCurrentRow(selectedIndex);
        }
    }
}

void CameraControlsPanel::onFormatSelected(int index)
{
    if (index >= 0 && index < _availableFormats.size()) {
        int cameraIndex = _cameraList->currentRow();
        if (cameraIndex >= 0 && cameraIndex < _availableCameras.size()) {
            QString cameraId = _availableCameras[cameraIndex].id();
            QCameraFormat format = _availableFormats[index];
            
            // Start camera with selected format
            _controller->startCamera(cameraId, format);
            
            // Apply fit-to-width zoom for new camera
            if (_zoomController) {
                _zoomController->setFitWidth();
            }
        }
    }
}

void CameraControlsPanel::updateGeometry(float baseTileSize, float sceneWidth, int fontSize)
{
    // Call base implementation first
    TileDialog::updateGeometry(baseTileSize, sceneWidth, fontSize);
}

void CameraControlsPanel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Draw base dialog background with title
    TileDialog::paint(painter, option, widget);
}
