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
    , _cameraCombo(nullptr)
    , _formatCombo(nullptr)
    , _contentWidget(nullptr)
    , _exposureSlider(nullptr)
    , _brightnessSlider(nullptr)
    , _contrastSlider(nullptr)
    , _saturationSlider(nullptr)
    , _autoWhiteBalanceBtn(nullptr)
    , _flipHorizontalBtn(nullptr)
    , _flipVerticalBtn(nullptr)
    , _resetDefaultsBtn(nullptr)
{
    setupUI();
}

void CameraControlsPanel::show(float baseTileSize, float sceneWidth)
{
    // Call base implementation to create and show the dialog
    TileDialog::show(baseTileSize, sceneWidth);
    
    // Reduce dim overlay opacity to 60 (very transparent) so user can see video changes
    // while adjusting camera controls
    setDimOpacity(60);
}

void CameraControlsPanel::setupUI()
{
    // Create content widget with standard Qt controls
    QWidget* content = new QWidget();
    content->setStyleSheet("QWidget { background-color: rgba(40, 40, 40, 220); }");
    
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    
    // Camera selection label
    QLabel* cameraLabel = new QLabel("Select Camera:");
    cameraLabel->setFixedHeight(20);
    cameraLabel->setStyleSheet("color: white; font-weight: bold;");
    
    // Camera combo box
    _cameraCombo = new QComboBox();
    _cameraCombo->setStyleSheet(
        "QComboBox {"
        "   background-color: rgba(60, 60, 60, 200);"
        "   color: white;"
        "   border: 2px solid rgba(100, 100, 100, 200);"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   min-height: 25px;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border-left: 5px solid transparent;"
        "   border-right: 5px solid transparent;"
        "   border-top: 5px solid white;"
        "   margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: rgba(60, 60, 60, 220);"
        "   color: white;"
        "   selection-background-color: rgba(80, 150, 80, 220);"
        "}"
    );
    connect(_cameraCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraControlsPanel::onCameraSelected);
    
    // Format selection label
    QLabel* formatLabel = new QLabel("Resolution & Frame Rate:");
    formatLabel->setFixedHeight(20);
    formatLabel->setStyleSheet("color: white; font-weight: bold;");
    
    // Format combo box
    _formatCombo = new QComboBox();
    _formatCombo->setStyleSheet(
        "QComboBox {"
        "   background-color: rgba(60, 60, 60, 200);"
        "   color: white;"
        "   border: 2px solid rgba(100, 100, 100, 200);"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   min-height: 25px;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   border-left: 5px solid transparent;"
        "   border-right: 5px solid transparent;"
        "   border-top: 5px solid white;"
        "   margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "   background-color: rgba(60, 60, 60, 220);"
        "   color: white;"
        "   selection-background-color: rgba(80, 150, 80, 220);"
        "}"
    );
    connect(_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraControlsPanel::onFormatSelected);
    
    // Exposure slider with value label
    _exposureLabel = new QLabel("Exposure: 100 ms");
    _exposureLabel->setStyleSheet("color: white;");
    _exposureLabel->setFixedHeight(20);
    _exposureSlider = new QSlider(Qt::Horizontal);
    _exposureSlider->setRange(10, 1000);  // 10-1000ms
    _exposureSlider->setValue(100);
    _exposureSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_exposureSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onExposureChanged);
    
    // Brightness slider with value label
    _brightnessLabel = new QLabel("Brightness: 0");
    _brightnessLabel->setStyleSheet("color: white;");
    _brightnessLabel->setFixedHeight(20);
    _brightnessSlider = new QSlider(Qt::Horizontal);
    _brightnessSlider->setRange(-100, 100);
    _brightnessSlider->setValue(0);
    _brightnessSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_brightnessSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onBrightnessChanged);
    
    // Contrast slider with value label
    _contrastLabel = new QLabel("Contrast: 0");
    _contrastLabel->setStyleSheet("color: white;");
    _contrastLabel->setFixedHeight(20);
    _contrastSlider = new QSlider(Qt::Horizontal);
    _contrastSlider->setRange(-100, 100);
    _contrastSlider->setValue(0);
    _contrastSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_contrastSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onContrastChanged);
    
    // Saturation slider with value label
    _saturationLabel = new QLabel("Saturation: 0");
    _saturationLabel->setStyleSheet("color: white;");
    _saturationLabel->setFixedHeight(20);
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
    
    _resetDefaultsBtn = new QPushButton("Reset Defaults");
    _resetDefaultsBtn->setStyleSheet(
        "QPushButton { background-color: rgba(150, 80, 80, 220); color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: rgba(170, 100, 100, 240); }"
        "QPushButton:pressed { background-color: rgba(130, 60, 60, 240); }"
    );
    connect(_resetDefaultsBtn, &QPushButton::clicked, this, &CameraControlsPanel::onResetDefaultsClicked);
    
    buttonLayout->addWidget(_autoWhiteBalanceBtn);
    buttonLayout->addWidget(_flipHorizontalBtn);
    buttonLayout->addWidget(_flipVerticalBtn);
    buttonLayout->addWidget(_resetDefaultsBtn);
    
    // Assemble layout
    layout->addWidget(cameraLabel);
    layout->addWidget(_cameraCombo);
    layout->addWidget(formatLabel);
    layout->addWidget(_formatCombo);
    layout->addWidget(_exposureLabel);
    layout->addWidget(_exposureSlider);
    layout->addWidget(_brightnessLabel);
    layout->addWidget(_brightnessSlider);
    layout->addWidget(_contrastLabel);
    layout->addWidget(_contrastSlider);
    layout->addWidget(_saturationLabel);
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
    
    _cameraCombo->clear();
    
    if (_availableCameras.isEmpty()) {
        _cameraCombo->addItem("No cameras detected");
    } else {
        int selectedRow = 0;
        QString currentCameraId = _controller->currentCameraId();
        
        for (int i = 0; i < _availableCameras.size(); ++i) {
            const CameraProfile& camera = _availableCameras[i];
            _cameraCombo->addItem(camera.name(), camera.id());
            
            // Select the currently active camera if there is one
            if (_controller->isActive() && camera.id() == currentCameraId) {
                selectedRow = i;
            }
        }
        
        _cameraCombo->setCurrentIndex(selectedRow);
    }
}

void CameraControlsPanel::onCameraSelected(int index)
{
    if (index >= 0 && index < _availableCameras.size()) {
        QString cameraId = _availableCameras[index].id();
        
        // Restore camera controls from settings for this camera
        _controller->restoreCameraControls(cameraId);
        
        // Update UI sliders to match restored values (without triggering save)
        _exposureSlider->blockSignals(true);
        _brightnessSlider->blockSignals(true);
        _contrastSlider->blockSignals(true);
        _saturationSlider->blockSignals(true);
        _flipHorizontalBtn->blockSignals(true);
        _flipVerticalBtn->blockSignals(true);
        
        _exposureSlider->setValue(static_cast<int>(_controller->exposure()));
        _brightnessSlider->setValue(_controller->brightness());
        _contrastSlider->setValue(_controller->contrast());
        _saturationSlider->setValue(_controller->saturation());
        _flipHorizontalBtn->setChecked(_controller->flipHorizontal());
        _flipVerticalBtn->setChecked(_controller->flipVertical());
        
        // Update labels
        _exposureLabel->setText(QString("Exposure: %1 ms").arg(_controller->exposure()));
        _brightnessLabel->setText(QString("Brightness: %1").arg(_controller->brightness()));
        _contrastLabel->setText(QString("Contrast: %1").arg(_controller->contrast()));
        _saturationLabel->setText(QString("Saturation: %1").arg(_controller->saturation()));
        
        _exposureSlider->blockSignals(false);
        _brightnessSlider->blockSignals(false);
        _contrastSlider->blockSignals(false);
        _saturationSlider->blockSignals(false);
        _flipHorizontalBtn->blockSignals(false);
        _flipVerticalBtn->blockSignals(false);
        
        // Get available formats for this camera
        _availableFormats = _controller->availableFormats(cameraId);
        
        // Populate format combo
        _formatCombo->clear();
        
        if (_availableFormats.isEmpty()) {
            _formatCombo->addItem("No formats available");
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
                
                _formatCombo->addItem(formatText);
                
                // Check if this matches the current format
                if (!currentFormat.isNull() &&
                    currentFormat.resolution() == resolution &&
                    qAbs(currentFormat.maxFrameRate() - fps) < 0.1) {
                    selectedIndex = i;
                }
            }
            
            // Select current format or first one
            _formatCombo->setCurrentIndex(selectedIndex);
        }
    }
}

void CameraControlsPanel::onFormatSelected(int index)
{
    if (index >= 0 && index < _availableFormats.size()) {
        int cameraIndex = _cameraCombo->currentIndex();
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
    // Custom paint with more transparency to see video through dialog
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    QRectF rect = boundingRect();
    qreal radius = rect.height() / 16.0;  // Rounded corners
    
    // Semi-transparent background (lower alpha to see video)
    painter->setBrush(QColor(60, 60, 60, 180));  // Reduced from 240 to 180
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect, radius, radius);
}

// Camera control slots (T100)
void CameraControlsPanel::onExposureChanged(int value)
{
    _exposureLabel->setText(QString("Exposure: %1 ms").arg(value));
    _controller->setExposure(static_cast<qreal>(value));
}

void CameraControlsPanel::onBrightnessChanged(int value)
{
    _brightnessLabel->setText(QString("Brightness: %1").arg(value));
    _controller->setBrightness(value);
}

void CameraControlsPanel::onContrastChanged(int value)
{
    _contrastLabel->setText(QString("Contrast: %1").arg(value));
    _controller->setContrast(value);
}

void CameraControlsPanel::onSaturationChanged(int value)
{
    _saturationLabel->setText(QString("Saturation: %1").arg(value));
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

void CameraControlsPanel::onResetDefaultsClicked()
{
    // Reset all sliders to default values
    _exposureSlider->setValue(100);      // Default exposure 100ms
    _brightnessSlider->setValue(0);      // Default brightness 0
    _contrastSlider->setValue(0);        // Default contrast 0
    _saturationSlider->setValue(0);      // Default saturation 0
    
    // Labels are updated automatically via valueChanged signals
    
    // Reset flip buttons
    _flipHorizontalBtn->setChecked(false);
    _flipVerticalBtn->setChecked(false);
    _controller->setFlipHorizontal(false);
    _controller->setFlipVertical(false);
    
    // Set auto white balance
    _controller->setWhiteBalance(static_cast<int>(QCamera::WhiteBalanceAuto));
}

