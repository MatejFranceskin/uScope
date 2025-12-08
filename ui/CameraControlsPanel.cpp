#include "CameraControlsPanel.h"
#include "../controllers/CameraController.h"
#include "../controllers/ZoomController.h"
#include <QPainter>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QCamera>
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
    , _exposureSlider(nullptr)
    , _brightnessSlider(nullptr)
    , _contrastSlider(nullptr)
    , _saturationSlider(nullptr)
    , _autoWhiteBalanceBtn(nullptr)
    , _flipHorizontalBtn(nullptr)
    , _flipVerticalBtn(nullptr)
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
    
    // Camera Controls Section
    QLabel* controlsLabel = new QLabel("Manual Controls:");
    controlsLabel->setFixedHeight(20);
    controlsLabel->setStyleSheet("color: white; font-weight: bold;");
    
    // Exposure slider
    QLabel* exposureLabel = new QLabel("Exposure:");
    exposureLabel->setStyleSheet("color: white;");
    _exposureSlider = new QSlider(Qt::Horizontal);
    _exposureSlider->setRange(10, 1000);  // 10-1000ms
    _exposureSlider->setValue(100);
    _exposureSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_exposureSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onExposureChanged);
    
    // Brightness slider
    QLabel* brightnessLabel = new QLabel("Brightness:");
    brightnessLabel->setStyleSheet("color: white;");
    _brightnessSlider = new QSlider(Qt::Horizontal);
    _brightnessSlider->setRange(-100, 100);
    _brightnessSlider->setValue(0);
    _brightnessSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_brightnessSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onBrightnessChanged);
    
    // Contrast slider
    QLabel* contrastLabel = new QLabel("Contrast:");
    contrastLabel->setStyleSheet("color: white;");
    _contrastSlider = new QSlider(Qt::Horizontal);
    _contrastSlider->setRange(-100, 100);
    _contrastSlider->setValue(0);
    _contrastSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_contrastSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onContrastChanged);
    
    // Saturation slider
    QLabel* saturationLabel = new QLabel("Saturation:");
    saturationLabel->setStyleSheet("color: white;");
    _saturationSlider = new QSlider(Qt::Horizontal);
    _saturationSlider->setRange(-100, 100);
    _saturationSlider->setValue(0);
    _saturationSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_saturationSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onSaturationChanged);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(5);
    
    _autoWhiteBalanceBtn = new QPushButton("Auto WB");
    _autoWhiteBalanceBtn->setStyleSheet(
        "QPushButton { background-color: rgba(80, 150, 80, 220); color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: rgba(100, 170, 100, 240); }"
        "QPushButton:pressed { background-color: rgba(60, 130, 60, 240); }"
    );
    connect(_autoWhiteBalanceBtn, &QPushButton::clicked, this, &CameraControlsPanel::onAutoWhiteBalanceClicked);
    
    _flipHorizontalBtn = new QPushButton("Flip H");
    _flipHorizontalBtn->setCheckable(true);
    _flipHorizontalBtn->setStyleSheet(
        "QPushButton { background-color: rgba(80, 80, 150, 220); color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: rgba(100, 100, 170, 240); }"
        "QPushButton:checked { background-color: rgba(80, 150, 80, 220); }"
    );
    connect(_flipHorizontalBtn, &QPushButton::clicked, this, &CameraControlsPanel::onFlipHorizontalClicked);
    
    _flipVerticalBtn = new QPushButton("Flip V");
    _flipVerticalBtn->setCheckable(true);
    _flipVerticalBtn->setStyleSheet(
        "QPushButton { background-color: rgba(80, 80, 150, 220); color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: rgba(100, 100, 170, 240); }"
        "QPushButton:checked { background-color: rgba(80, 150, 80, 220); }"
    );
    connect(_flipVerticalBtn, &QPushButton::clicked, this, &CameraControlsPanel::onFlipVerticalClicked);
    
    buttonLayout->addWidget(_autoWhiteBalanceBtn);
    buttonLayout->addWidget(_flipHorizontalBtn);
    buttonLayout->addWidget(_flipVerticalBtn);
    
    // Assemble layout
    layout->addWidget(cameraLabel);
    layout->addWidget(_cameraList);
    layout->addWidget(formatLabel);
    layout->addWidget(_formatList);
    layout->addWidget(controlsLabel);
    layout->addWidget(exposureLabel);
    layout->addWidget(_exposureSlider);
    layout->addWidget(brightnessLabel);
    layout->addWidget(_brightnessSlider);
    layout->addWidget(contrastLabel);
    layout->addWidget(_contrastSlider);
    layout->addWidget(saturationLabel);
    layout->addWidget(_saturationSlider);
    layout->addLayout(buttonLayout);
    
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
        int selectedRow = 0;
        QString currentCameraId = _controller->currentCameraId();
        
        for (int i = 0; i < _availableCameras.size(); ++i) {
            const CameraProfile& camera = _availableCameras[i];
            QListWidgetItem* item = new QListWidgetItem(camera.name());
            item->setData(Qt::UserRole, camera.id());
            _cameraList->addItem(item);
            
            // Select the currently active camera if there is one
            if (_controller->isActive() && camera.id() == currentCameraId) {
                selectedRow = i;
            }
        }
        _cameraList->setCurrentRow(selectedRow);
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
            
            // Check if this is already the current camera and format
            if (_controller->isActive() && 
                _controller->currentCameraId() == cameraId &&
                !_controller->currentFormat().isNull()) {
                QCameraFormat currentFormat = _controller->currentFormat();
                if (currentFormat.resolution() == format.resolution() &&
                    qAbs(currentFormat.maxFrameRate() - format.maxFrameRate()) < 0.1) {
                    // Already on this camera and format, don't restart or save
                    qDebug() << "CameraControlsPanel::onFormatSelected - already on this camera/format, skipping";
                    return;
                }
            }
            
            // User is changing to a different camera/format - save and start
            qDebug() << "CameraControlsPanel::onFormatSelected - switching to camera:" << cameraId << "format:" << format.resolution();
            
            // Save the user's selection immediately
            _controller->saveCameraSelection(cameraId, format);
            
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

// Camera control slots (T100)
void CameraControlsPanel::onExposureChanged(int value)
{
    _controller->setExposure(static_cast<qreal>(value));
}

void CameraControlsPanel::onBrightnessChanged(int value)
{
    _controller->setBrightness(value);
}

void CameraControlsPanel::onContrastChanged(int value)
{
    _controller->setContrast(value);
}

void CameraControlsPanel::onSaturationChanged(int value)
{
    _controller->setSaturation(value);
}

void CameraControlsPanel::onAutoWhiteBalanceClicked()
{
    // Auto white balance - set to automatic mode
    _controller->setWhiteBalance(static_cast<int>(QCamera::WhiteBalanceAuto));
}

void CameraControlsPanel::onFlipHorizontalClicked()
{
    _controller->setFlipHorizontal(_flipHorizontalBtn->isChecked());
}

void CameraControlsPanel::onFlipVerticalClicked()
{
    _controller->setFlipVertical(_flipVerticalBtn->isChecked());
}

