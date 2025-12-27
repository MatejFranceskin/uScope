#include "CameraControlsPanel.h"
#include "../controllers/CameraController.h"
#include "../controllers/ZoomController.h"
#include <QPainter>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
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
    , _resolutionCombo(nullptr)
    , _fpsCombo(nullptr)
    , _resolutionLabel(nullptr)
    , _fpsLabel(nullptr)
    , _contentWidget(nullptr)
    , _exposureSlider(nullptr)
    , _brightnessSlider(nullptr)
    , _contrastSlider(nullptr)
    , _saturationSlider(nullptr)
    , _whiteBalanceSlider(nullptr)
    , _autoExposureCheck(nullptr)
    , _autoWhiteBalanceCheck(nullptr)
    , _autoExposureEnabled(true)
    , _autoWhiteBalanceEnabled(true)
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
    QLabel* cameraLabel = new QLabel("Camera:");
    cameraLabel->setStyleSheet("color: white;");
    
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
    
    // Format combo box
    _resolutionCombo = new QComboBox();
    _resolutionCombo->setStyleSheet(
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
    connect(_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraControlsPanel::onResolutionSelected);
    
    _fpsCombo = new QComboBox();
    _fpsCombo->setStyleSheet(
        "QComboBox { "
        "   background-color: rgba(60, 60, 60, 200); "
        "   color: white; "
        "   border: 1px solid rgba(80, 80, 80, 200); "
        "   border-radius: 4px; "
        "   padding: 4px; "
        "   min-width: 100px; "
        "} "
        "QComboBox:hover { "
        "   background-color: rgba(70, 70, 70, 200); "
        "   border: 1px solid rgba(100, 100, 100, 200); "
        "} "
        "QComboBox::drop-down { "
        "   border: none; "
        "} "
        "QComboBox::down-arrow { "
        "   image: url(:/icons/down-arrow.png); "
        "   width: 12px; "
        "   height: 12px; "
        "} "
        "QComboBox QAbstractItemView { "
        "   background-color: rgba(60, 60, 60, 240); "
        "   color: white; "
        "   selection-background-color: rgba(0, 120, 215, 200); "
        "   border: 1px solid rgba(80, 80, 80, 200); "
        "} "
    );
    connect(_fpsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraControlsPanel::onFpsSelected);
    
    // Exposure slider with separate name and value labels (hidden when auto exposure is on)
    _exposureLabel = new QLabel("Exposure:");
    _exposureLabel->setStyleSheet("color: white;");
    _exposureLabel->setVisible(false);  // Hidden when auto exposure is on
    _exposureValueLabel = new QLabel("100 ms");
    _exposureValueLabel->setStyleSheet("color: white;");
    _exposureValueLabel->setVisible(false);  // Hidden when auto exposure is on
    _exposureSlider = new QSlider(Qt::Horizontal);
    _exposureSlider->setRange(10, 1000);  // 10-1000ms
    _exposureSlider->setValue(100);
    _exposureSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    _exposureSlider->setVisible(false);  // Hidden when auto exposure is on
    connect(_exposureSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onExposureChanged);
    
    // Auto exposure checkbox
    _autoExposureCheck = new QCheckBox();
    _autoExposureCheck->setStyleSheet(
        "QCheckBox::indicator { "
        "   width: 18px; "
        "   height: 18px; "
        "   border: 2px solid rgba(100, 100, 100, 200); "
        "   border-radius: 3px; "
        "   background-color: rgba(60, 60, 60, 200); "
        "} "
        "QCheckBox::indicator:checked { "
        "   background-color: rgba(80, 150, 80, 220); "
        "   border: 2px solid rgba(80, 150, 80, 220); "
        "} "
        "QCheckBox::indicator:hover { "
        "   border: 2px solid rgba(120, 120, 120, 220); "
        "}"
    );
    _autoExposureCheck->setChecked(true);  // On by default
    connect(_autoExposureCheck, &QCheckBox::toggled, this, &CameraControlsPanel::onAutoExposureClicked);
    
    // Brightness slider with separate name and value labels
    _brightnessLabel = new QLabel("Brightness:");
    _brightnessLabel->setStyleSheet("color: white;");
    _brightnessValueLabel = new QLabel("128");
    _brightnessValueLabel->setStyleSheet("color: white;");
    _brightnessSlider = new QSlider(Qt::Horizontal);
    _brightnessSlider->setRange(0, 255);  // V4L2 brightness range
    _brightnessSlider->setValue(128);     // Default to middle
    _brightnessSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_brightnessSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onBrightnessChanged);
    
    // Contrast slider with separate name and value labels
    _contrastLabel = new QLabel("Contrast:");
    _contrastLabel->setStyleSheet("color: white;");
    _contrastValueLabel = new QLabel("32");
    _contrastValueLabel->setStyleSheet("color: white;");
    _contrastSlider = new QSlider(Qt::Horizontal);
    _contrastSlider->setRange(0, 100);  // V4L2 contrast range
    _contrastSlider->setValue(32);      // Default
    _contrastSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_contrastSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onContrastChanged);
    
    // Saturation slider with separate name and value labels
    _saturationLabel = new QLabel("Saturation:");
    _saturationLabel->setStyleSheet("color: white;");
    _saturationValueLabel = new QLabel("64");
    _saturationValueLabel->setStyleSheet("color: white;");
    _saturationSlider = new QSlider(Qt::Horizontal);
    _saturationSlider->setRange(0, 100);  // V4L2 saturation range
    _saturationSlider->setValue(64);      // Default
    _saturationSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    connect(_saturationSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onSaturationChanged);
    
    // Auto white balance checkbox
    _autoWhiteBalanceCheck = new QCheckBox();
    _autoWhiteBalanceCheck->setStyleSheet(
        "QCheckBox::indicator { "
        "   width: 18px; "
        "   height: 18px; "
        "   border: 2px solid rgba(100, 100, 100, 200); "
        "   border-radius: 3px; "
        "   background-color: rgba(60, 60, 60, 200); "
        "} "
        "QCheckBox::indicator:checked { "
        "   background-color: rgba(80, 150, 80, 220); "
        "   border: 2px solid rgba(80, 150, 80, 220); "
        "} "
        "QCheckBox::indicator:hover { "
        "   border: 2px solid rgba(120, 120, 120, 220); "
        "}"
    );
    _autoWhiteBalanceCheck->setChecked(true);  // On by default
    connect(_autoWhiteBalanceCheck, &QCheckBox::toggled, this, &CameraControlsPanel::onAutoWhiteBalanceClicked);
    
    // White balance temperature slider (2800-6500K, hidden by default when auto WB is on)
    _whiteBalanceLabel = new QLabel("WB Temp:");
    _whiteBalanceLabel->setStyleSheet("color: white;");
    _whiteBalanceLabel->setVisible(false);  // Hidden when auto WB is on
    _whiteBalanceValueLabel = new QLabel("4600K");
    _whiteBalanceValueLabel->setStyleSheet("color: white;");
    _whiteBalanceValueLabel->setVisible(false);  // Hidden when auto WB is on
    _whiteBalanceSlider = new QSlider(Qt::Horizontal);
    _whiteBalanceSlider->setRange(2800, 6500);
    _whiteBalanceSlider->setValue(4600);
    _whiteBalanceSlider->setStyleSheet(
        "QSlider::groove:horizontal { background: rgba(100, 100, 100, 200); height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: rgba(80, 150, 80, 220); width: 12px; margin: -4px 0; border-radius: 6px; }"
    );
    _whiteBalanceSlider->setVisible(false);  // Hidden when auto WB is on
    connect(_whiteBalanceSlider, &QSlider::valueChanged, this, &CameraControlsPanel::onWhiteBalanceChanged);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(5);
    
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
    
    _resetDefaultsBtn = new QPushButton("Default");
    _resetDefaultsBtn->setStyleSheet(
        "QPushButton { background-color: rgba(150, 80, 80, 220); color: white; border-radius: 5px; padding: 5px; }"
        "QPushButton:hover { background-color: rgba(170, 100, 100, 240); }"
        "QPushButton:pressed { background-color: rgba(130, 60, 60, 240); }"
    );
    connect(_resetDefaultsBtn, &QPushButton::clicked, this, &CameraControlsPanel::onResetDefaultsClicked);
    
    buttonLayout->addWidget(_flipHorizontalBtn);
    buttonLayout->addWidget(_flipVerticalBtn);
    buttonLayout->addWidget(_resetDefaultsBtn);
    
    // Create grid layout for automatic column alignment
    // Columns: 0=label, 1=value, 2=control
    QGridLayout* controlsGrid = new QGridLayout();
    controlsGrid->setColumnStretch(0, 0);  // Label column: no stretch
    controlsGrid->setColumnStretch(1, 0);  // Value column: no stretch  
    controlsGrid->setColumnStretch(2, 1);  // Control column: stretch to fill
    controlsGrid->setHorizontalSpacing(12);  // Space between label/value and control
    controlsGrid->setVerticalSpacing(12);  // Half of control height (~25px controls)
    
    int row = 0;
    
    // Camera selection (spans value column since no value label)
    controlsGrid->addWidget(cameraLabel, row, 0, Qt::AlignLeft);
    controlsGrid->addWidget(_cameraCombo, row, 1, 1, 2);  // Span columns 1-2
    row++;
    
    // Resolution selection (spans value column since no value label)
    _resolutionLabel = new QLabel("Resolution:");
    _resolutionLabel->setStyleSheet("color: white;");
    controlsGrid->addWidget(_resolutionLabel, row, 0, Qt::AlignLeft);
    controlsGrid->addWidget(_resolutionCombo, row, 1, 1, 2);  // Span columns 1-2
    row++;
    
    // Frame rate selection (spans value column since no value label)
    _fpsLabel = new QLabel("Frame Rate:");
    _fpsLabel->setStyleSheet("color: white;");
    controlsGrid->addWidget(_fpsLabel, row, 0, Qt::AlignLeft);
    controlsGrid->addWidget(_fpsCombo, row, 1, 1, 2);  // Span columns 1-2
    row++;
    
    // Create container widgets for UVC and PTP controls
    _uvcControlsWidget = new QWidget();
    QGridLayout* uvcLayout = new QGridLayout(_uvcControlsWidget);
    uvcLayout->setColumnStretch(0, 0);
    uvcLayout->setColumnStretch(1, 0);
    uvcLayout->setColumnStretch(2, 1);
    uvcLayout->setHorizontalSpacing(12);
    uvcLayout->setVerticalSpacing(12);
    uvcLayout->setContentsMargins(0, 0, 0, 0);
    
    int uvcRow = 0;
    setupUVCControls(uvcLayout, uvcRow);
    
    _ptpControlsWidget = new QWidget();
    QGridLayout* ptpLayout = new QGridLayout(_ptpControlsWidget);
    ptpLayout->setColumnStretch(0, 0);
    ptpLayout->setColumnStretch(1, 0);
    ptpLayout->setColumnStretch(2, 1);
    ptpLayout->setHorizontalSpacing(12);
    ptpLayout->setVerticalSpacing(12);
    ptpLayout->setContentsMargins(0, 0, 0, 0);
    
    int ptpRow = 0;
    setupPTPControls(ptpLayout, ptpRow);
    
    // Initially hide PTP controls
    _ptpControlsWidget->setVisible(false);
    
    // Add both control widgets to grid (they occupy the same rows)
    controlsGrid->addWidget(_uvcControlsWidget, row, 0, 1, 3);
    controlsGrid->addWidget(_ptpControlsWidget, row, 0, 1, 3);
    row++;
    
    // Add grid to main layout
    layout->addLayout(controlsGrid);
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
        
        // Manually trigger camera selection to populate resolutions and FPS
        // (setCurrentIndex doesn't always trigger the signal)
        onCameraSelected(selectedRow);
    }
}

void CameraControlsPanel::onCameraSelected(int index)
{
    qDebug() << "CameraControlsPanel::onCameraSelected - index:" << index;
    
    if (index >= 0 && index < _availableCameras.size()) {
        QString cameraId = _availableCameras[index].id();
        
        qDebug() << "CameraControlsPanel::onCameraSelected - cameraId:" << cameraId;
        
        // Update control visibility based on camera type
        updateControlsVisibility();
        
        // Restore camera controls from settings for this camera
        _controller->restoreCameraControls(cameraId);
        
        // Update UI sliders to match restored values (without triggering save)
        _exposureSlider->blockSignals(true);
        _brightnessSlider->blockSignals(true);
        _contrastSlider->blockSignals(true);
        _saturationSlider->blockSignals(true);
        _whiteBalanceSlider->blockSignals(true);
        _autoExposureCheck->blockSignals(true);
        _autoWhiteBalanceCheck->blockSignals(true);
        _flipHorizontalBtn->blockSignals(true);
        _flipVerticalBtn->blockSignals(true);
        
        _exposureSlider->setValue(static_cast<int>(_controller->exposure()));
        _brightnessSlider->setValue(_controller->brightness());
        _contrastSlider->setValue(_controller->contrast());
        _saturationSlider->setValue(_controller->saturation());
        _whiteBalanceSlider->setValue(_controller->whiteBalance());
        _autoExposureCheck->setChecked(_controller->autoExposure());
        _autoWhiteBalanceCheck->setChecked(_controller->autoWhiteBalance());
        _flipHorizontalBtn->setChecked(_controller->flipHorizontal());
        _flipVerticalBtn->setChecked(_controller->flipVertical());
        
        // Update value labels
        _exposureValueLabel->setText(QString("%1 ms").arg(_controller->exposure()));
        _brightnessValueLabel->setText(QString("%1").arg(_controller->brightness()));
        _contrastValueLabel->setText(QString("%1").arg(_controller->contrast()));
        _saturationValueLabel->setText(QString("%1").arg(_controller->saturation()));
        _whiteBalanceValueLabel->setText(QString("%1K").arg(_controller->whiteBalance()));
        
        // Update visibility of exposure and white balance sliders based on auto modes
        _autoExposureEnabled = _controller->autoExposure();
        _autoWhiteBalanceEnabled = _controller->autoWhiteBalance();
        _exposureLabel->setVisible(!_autoExposureEnabled);
        _exposureValueLabel->setVisible(!_autoExposureEnabled);
        _exposureSlider->setVisible(!_autoExposureEnabled);
        _whiteBalanceLabel->setVisible(!_autoWhiteBalanceEnabled);
        _whiteBalanceValueLabel->setVisible(!_autoWhiteBalanceEnabled);
        _whiteBalanceSlider->setVisible(!_autoWhiteBalanceEnabled);
        
        _exposureSlider->blockSignals(false);
        _brightnessSlider->blockSignals(false);
        _contrastSlider->blockSignals(false);
        _saturationSlider->blockSignals(false);
        _whiteBalanceSlider->blockSignals(false);
        _autoExposureCheck->blockSignals(false);
        _autoWhiteBalanceCheck->blockSignals(false);
        _flipHorizontalBtn->blockSignals(false);
        _flipVerticalBtn->blockSignals(false);
        
        // Get available resolutions for this camera
        _availableResolutions = _controller->availableResolutions(cameraId);
        
        // Block resolution combo signals while populating
        _resolutionCombo->blockSignals(true);
        
        // Populate resolution combo
        _resolutionCombo->clear();
        
        int selectedIndex = 0;  // Declare outside if-else for later use
        
        if (_availableResolutions.isEmpty()) {
            // PTP cameras don't have resolution selection
            _resolutionCombo->addItem("Auto");
            _fpsCombo->clear();
            _fpsCombo->addItem("Auto");
            
            // For PTP cameras, start immediately without resolution/fps selection
            bool needsCameraSwitch = !_controller->isActive() || 
                                     _controller->currentCameraId() != cameraId;
            
            if (needsCameraSwitch) {
                qDebug() << "CameraControlsPanel::onCameraSelected - starting PTP camera:" << cameraId;
                _controller->startCamera(cameraId);
                _controller->saveCameraSelection(cameraId, QSize());
                
                // Apply fit-to-width zoom for new camera
                if (_zoomController) {
                    _zoomController->setFitWidth();
                }
                
                // Delay capability fetch to allow camera to initialize
                QTimer::singleShot(500, this, [this]() {
                    updateControlsVisibility();
                });
            } else {
                // Camera already active, just update controls
                updateControlsVisibility();
            }
        } else {
            // Sort resolutions: higher resolution first
            std::sort(_availableResolutions.begin(), _availableResolutions.end(), 
                [](const QSize& a, const QSize& b) {
                    int pixelsA = a.width() * a.height();
                    int pixelsB = b.width() * b.height();
                    return pixelsA > pixelsB;  // Higher resolution first
                });
            
            // Get saved resolution for this camera, or current resolution if active
            QSize savedResolution = _controller->getSavedResolution(cameraId);
            QSize currentResolution = _controller->currentResolution();
            QSize preferredResolution = savedResolution.isValid() ? savedResolution : currentResolution;
            
            for (int i = 0; i < _availableResolutions.size(); ++i) {
                const QSize& resolution = _availableResolutions[i];
                
                QString formatText = QString("%1x%2")
                    .arg(resolution.width())
                    .arg(resolution.height());
                
                _resolutionCombo->addItem(formatText);
                
                // Check if this matches the preferred resolution
                if (preferredResolution.isValid() &&
                    preferredResolution == resolution) {
                    selectedIndex = i;
                }
            }
            
            // Select preferred resolution or first one
            _resolutionCombo->setCurrentIndex(selectedIndex);
        }
        
        // Unblock resolution combo signals
        _resolutionCombo->blockSignals(false);
        
        // Populate FPS combo for the selected resolution and start camera if needed
        if (selectedIndex >= 0 && selectedIndex < _availableResolutions.size()) {
            // Check if we need to switch cameras
            bool needsCameraSwitch = !_controller->isActive() || 
                                     _controller->currentCameraId() != cameraId;
            
            if (needsCameraSwitch) {
                // Populate FPS combo then start the camera
                onResolutionSelected(selectedIndex);
                // onResolutionSelected will call onFpsSelected which starts the camera
            } else {
                // Just update the FPS combo, don't restart camera
                onResolutionSelected(selectedIndex);
            }
        }
    }
}

void CameraControlsPanel::onResolutionSelected(int index)
{
    if (index >= 0 && index < _availableResolutions.size()) {
        QSize resolution = _availableResolutions[index];
        
        // Update frame rate combo for this resolution
        int cameraIndex = _cameraCombo->currentIndex();
        if (cameraIndex >= 0 && cameraIndex < _availableCameras.size()) {
            QString cameraId = _availableCameras[cameraIndex].id();
            
            _fpsCombo->blockSignals(true);
            _fpsCombo->clear();
            
            _availableFrameRates = _controller->availableFrameRates(cameraId, resolution);
            
            if (_availableFrameRates.isEmpty()) {
                _fpsCombo->addItem("No frame rates available");
            } else {
                double currentFps = _controller->currentFrameRate();
                int selectedFpsIndex = 0;
                
                for (int i = 0; i < _availableFrameRates.size(); ++i) {
                    double fps = _availableFrameRates[i];
                    QString fpsText = QString("%1 fps").arg(fps, 0, 'f', fps == static_cast<int>(fps) ? 0 : 1);
                    _fpsCombo->addItem(fpsText);
                    
                    if (qAbs(currentFps - fps) < 0.1) {
                        selectedFpsIndex = i;
                    }
                }
                
                _fpsCombo->setCurrentIndex(selectedFpsIndex);
            }
            
            _fpsCombo->blockSignals(false);
            
            // Trigger FPS selection to apply the change
            // (only if FPS combo has valid selection)
            if (_fpsCombo->currentIndex() >= 0 && !_availableFrameRates.isEmpty()) {
                onFpsSelected(_fpsCombo->currentIndex());
            }
        }
    }
}

void CameraControlsPanel::onFpsSelected(int index)
{
    if (index >= 0 && index < _availableFrameRates.size()) {
        int cameraIndex = _cameraCombo->currentIndex();
        int resIndex = _resolutionCombo->currentIndex();
        
        if (cameraIndex >= 0 && cameraIndex < _availableCameras.size() &&
            resIndex >= 0 && resIndex < _availableResolutions.size()) {
            
            QString cameraId = _availableCameras[cameraIndex].id();
            QSize resolution = _availableResolutions[resIndex];
            double frameRate = _availableFrameRates[index];
            
            // Check if this is already the current camera, resolution and frame rate
            if (_controller->isActive() && 
                _controller->currentCameraId() == cameraId) {
                
                // Same camera - check if resolution and fps are also the same
                if (_controller->currentResolution() == resolution) {
                    double currentFps = _controller->currentFrameRate();
                    if (qAbs(currentFps - frameRate) < 0.1) {
                        // Already on this camera/resolution/fps, don't restart
                        qDebug() << "CameraControlsPanel::onFpsSelected - already on this configuration, skipping";
                        return;
                    }
                }
            }
            
            // User is changing configuration - save and start
            qDebug() << "CameraControlsPanel::onFpsSelected - switching to camera:" << cameraId << "resolution:" << resolution << "@" << frameRate << "fps";
            
            // Save the user's selection
            _controller->saveCameraSelection(cameraId, resolution);
            
            // Start camera with selected resolution and frame rate
            _controller->startCamera(cameraId, resolution, frameRate);
            
            // Save camera controls (including resolution) for this camera
            _controller->saveCameraControls(cameraId);
            
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
    _exposureValueLabel->setText(QString("%1 ms").arg(value));
    _controller->setExposure(static_cast<qreal>(value));
}

void CameraControlsPanel::onBrightnessChanged(int value)
{
    _brightnessValueLabel->setText(QString("%1").arg(value));
    _controller->setBrightness(value);
}

void CameraControlsPanel::onContrastChanged(int value)
{
    _contrastValueLabel->setText(QString("%1").arg(value));
    _controller->setContrast(value);
}

void CameraControlsPanel::onSaturationChanged(int value)
{
    _saturationValueLabel->setText(QString("%1").arg(value));
    _controller->setSaturation(value);
}

void CameraControlsPanel::onWhiteBalanceChanged(int value)
{
    _whiteBalanceValueLabel->setText(QString("%1K").arg(value));
    _controller->setWhiteBalance(value);
}

void CameraControlsPanel::onAutoExposureClicked()
{
    // Get checkbox state
    _autoExposureEnabled = _autoExposureCheck->isChecked();
    _controller->setAutoExposure(_autoExposureEnabled);
    
    // If disabling auto mode, set to a reasonable default value
    // (Camera doesn't expose the actual auto exposure value via OpenCV)
    if (!_autoExposureEnabled) {
        int defaultExposure = 100;  // 100ms is a good starting point
        _exposureSlider->blockSignals(true);
        _exposureSlider->setValue(defaultExposure);
        _exposureValueLabel->setText(QString("%1 ms").arg(defaultExposure));
        _exposureSlider->blockSignals(false);
        
        // Apply the value to ensure camera uses it
        _controller->setExposure(static_cast<qreal>(defaultExposure));
    }
    
    // Show/hide exposure slider (visible when auto is OFF)
    _exposureLabel->setVisible(!_autoExposureEnabled);
    _exposureValueLabel->setVisible(!_autoExposureEnabled);
    _exposureSlider->setVisible(!_autoExposureEnabled);
}

void CameraControlsPanel::onAutoWhiteBalanceClicked()
{
    // Get checkbox state
    _autoWhiteBalanceEnabled = _autoWhiteBalanceCheck->isChecked();
    _controller->setAutoWhiteBalance(_autoWhiteBalanceEnabled);
    
    // If disabling auto mode, set to a reasonable default value
    // (Camera doesn't expose the actual auto WB value via OpenCV)
    if (!_autoWhiteBalanceEnabled) {
        int defaultWhiteBalance = 4600;  // 4600K is neutral daylight
        _whiteBalanceSlider->blockSignals(true);
        _whiteBalanceSlider->setValue(defaultWhiteBalance);
        _whiteBalanceValueLabel->setText(QString("%1K").arg(defaultWhiteBalance));
        _whiteBalanceSlider->blockSignals(false);
        
        // Apply the value to ensure camera uses it
        _controller->setWhiteBalance(defaultWhiteBalance);
    }
    
    // Show/hide white balance temperature slider (visible when auto is OFF)
    _whiteBalanceLabel->setVisible(!_autoWhiteBalanceEnabled);
    _whiteBalanceValueLabel->setVisible(!_autoWhiteBalanceEnabled);
    _whiteBalanceSlider->setVisible(!_autoWhiteBalanceEnabled);
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
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - starting";
    
    // Block signals to avoid triggering handlers during reset
    if (_cameraCombo) _cameraCombo->blockSignals(true);
    if (_resolutionCombo) _resolutionCombo->blockSignals(true);
    if (_fpsCombo) _fpsCombo->blockSignals(true);
    if (_autoExposureCheck) _autoExposureCheck->blockSignals(true);
    if (_autoWhiteBalanceCheck) _autoWhiteBalanceCheck->blockSignals(true);
    if (_exposureSlider) _exposureSlider->blockSignals(true);
    if (_brightnessSlider) _brightnessSlider->blockSignals(true);
    if (_contrastSlider) _contrastSlider->blockSignals(true);
    if (_saturationSlider) _saturationSlider->blockSignals(true);
    if (_whiteBalanceSlider) _whiteBalanceSlider->blockSignals(true);
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - signals blocked";
    
    // Re-enable auto modes for best default behavior
    _autoExposureEnabled = true;
    _autoWhiteBalanceEnabled = true;
    
    // Update checkboxes
    if (_autoExposureCheck) _autoExposureCheck->setChecked(true);
    if (_autoWhiteBalanceCheck) _autoWhiteBalanceCheck->setChecked(true);
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - checkboxes updated";
    
    // Hide exposure and white balance sliders (auto modes are on)
    if (_exposureLabel) _exposureLabel->setVisible(false);
    if (_exposureValueLabel) _exposureValueLabel->setVisible(false);
    if (_exposureSlider) _exposureSlider->setVisible(false);
    if (_whiteBalanceLabel) _whiteBalanceLabel->setVisible(false);
    if (_whiteBalanceValueLabel) _whiteBalanceValueLabel->setVisible(false);
    if (_whiteBalanceSlider) _whiteBalanceSlider->setVisible(false);
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - sliders hidden";
    
    // Reset all sliders to default values
    if (_exposureSlider) _exposureSlider->setValue(100);      // Default exposure 100ms
    if (_brightnessSlider) _brightnessSlider->setValue(128);    // Default brightness (middle of 0-255)
    if (_contrastSlider) _contrastSlider->setValue(32);       // Default contrast
    if (_saturationSlider) _saturationSlider->setValue(64);     // Default saturation
    if (_whiteBalanceSlider) _whiteBalanceSlider->setValue(4600); // Default WB temperature
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - slider values set";
    
    // Update value labels manually since signals are blocked
    if (_exposureValueLabel) _exposureValueLabel->setText("100 ms");
    if (_brightnessValueLabel) _brightnessValueLabel->setText("128");
    if (_contrastValueLabel) _contrastValueLabel->setText("32");
    if (_saturationValueLabel) _saturationValueLabel->setText("64");
    if (_whiteBalanceValueLabel) _whiteBalanceValueLabel->setText("4600K");
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - labels updated";
    
    // Reset flip buttons
    if (_flipHorizontalBtn) _flipHorizontalBtn->setChecked(false);
    if (_flipVerticalBtn) _flipVerticalBtn->setChecked(false);
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - flip buttons reset";
    
    // Unblock signals
    if (_cameraCombo) _cameraCombo->blockSignals(false);
    if (_resolutionCombo) _resolutionCombo->blockSignals(false);
    if (_fpsCombo) _fpsCombo->blockSignals(false);
    if (_autoExposureCheck) _autoExposureCheck->blockSignals(false);
    if (_autoWhiteBalanceCheck) _autoWhiteBalanceCheck->blockSignals(false);
    if (_exposureSlider) _exposureSlider->blockSignals(false);
    if (_brightnessSlider) _brightnessSlider->blockSignals(false);
    if (_contrastSlider) _contrastSlider->blockSignals(false);
    if (_saturationSlider) _saturationSlider->blockSignals(false);
    if (_whiteBalanceSlider) _whiteBalanceSlider->blockSignals(false);
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - signals unblocked";
    
    // Now apply settings to controller
    if (_controller) {
        qDebug() << "CameraControlsPanel::onResetDefaultsClicked - applying to controller";
        _controller->setAutoExposure(true);
        _controller->setAutoWhiteBalance(true);
        _controller->setBrightness(128);
        _controller->setContrast(32);
        _controller->setSaturation(64);
        _controller->setFlipHorizontal(false);
        _controller->setFlipVertical(false);
        qDebug() << "CameraControlsPanel::onResetDefaultsClicked - controller updated";
    }
    
    qDebug() << "CameraControlsPanel::onResetDefaultsClicked - completed";
}

void CameraControlsPanel::setupUVCControls(QGridLayout* layout, int& row)
{
    // Auto exposure checkbox (spans value column)
    QLabel* autoExposureLabel = new QLabel("Auto Exp:");
    autoExposureLabel->setStyleSheet("color: white;");
    layout->addWidget(autoExposureLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_autoExposureCheck, row, 1, Qt::AlignLeft);
    row++;
    
    // Exposure slider with value label
    layout->addWidget(_exposureLabel, row, 0, Qt::AlignLeft);
    _exposureValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_exposureValueLabel, row, 1, Qt::AlignRight);
    layout->addWidget(_exposureSlider, row, 2);
    row++;
    
    // Brightness slider with value label
    layout->addWidget(_brightnessLabel, row, 0, Qt::AlignLeft);
    _brightnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_brightnessValueLabel, row, 1, Qt::AlignRight);
    layout->addWidget(_brightnessSlider, row, 2);
    row++;
    
    // Contrast slider with value label
    layout->addWidget(_contrastLabel, row, 0, Qt::AlignLeft);
    _contrastValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_contrastValueLabel, row, 1, Qt::AlignRight);
    layout->addWidget(_contrastSlider, row, 2);
    row++;
    
    // Saturation slider with value label
    layout->addWidget(_saturationLabel, row, 0, Qt::AlignLeft);
    _saturationValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_saturationValueLabel, row, 1, Qt::AlignRight);
    layout->addWidget(_saturationSlider, row, 2);
    row++;
    
    // Auto white balance checkbox (spans value column)
    QLabel* autoWhiteBalanceLabel = new QLabel("Auto WB:");
    autoWhiteBalanceLabel->setStyleSheet("color: white;");
    layout->addWidget(autoWhiteBalanceLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_autoWhiteBalanceCheck, row, 1, Qt::AlignLeft);
    row++;
    
    // White balance slider with value label
    layout->addWidget(_whiteBalanceLabel, row, 0, Qt::AlignLeft);
    _whiteBalanceValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(_whiteBalanceValueLabel, row, 1, Qt::AlignRight);
    layout->addWidget(_whiteBalanceSlider, row, 2);
    row++;
}

void CameraControlsPanel::setupPTPControls(QGridLayout* layout, int& row)
{
    // Exposure Mode (Aperture Priority or Manual only for microscopy)
    _ptpExposureModeLabel = new QLabel("Exp Mode:");
    _ptpExposureModeLabel->setStyleSheet("color: white;");
    _ptpExposureModeCombo = new QComboBox();
    _ptpExposureModeCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    layout->addWidget(_ptpExposureModeLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpExposureModeCombo, row, 1, 1, 2);
    row++;
    
    // ISO
    _ptpIsoLabel = new QLabel("ISO:");
    _ptpIsoLabel->setStyleSheet("color: white;");
    _ptpIsoCombo = new QComboBox();
    _ptpIsoCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    layout->addWidget(_ptpIsoLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpIsoCombo, row, 1, 1, 2);
    row++;
    
    // Shutter Speed (only visible in Manual mode)
    _ptpShutterSpeedLabel = new QLabel("Shutter:");
    _ptpShutterSpeedLabel->setStyleSheet("color: white;");
    _ptpShutterSpeedCombo = new QComboBox();
    _ptpShutterSpeedCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    layout->addWidget(_ptpShutterSpeedLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpShutterSpeedCombo, row, 1, 1, 2);
    row++;
    
    // Aperture - hidden for microscopy (aperture is fixed)
    _ptpApertureLabel = new QLabel("Aperture:");
    _ptpApertureLabel->setStyleSheet("color: white;");
    _ptpApertureLabel->setVisible(false);
    _ptpApertureCombo = new QComboBox();
    _ptpApertureCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    _ptpApertureCombo->setVisible(false);
    layout->addWidget(_ptpApertureLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpApertureCombo, row, 1, 1, 2);
    row++;
    
    // White Balance Mode
    _ptpWhiteBalanceLabel = new QLabel("WB Mode:");
    _ptpWhiteBalanceLabel->setStyleSheet("color: white;");
    _ptpWhiteBalanceCombo = new QComboBox();
    _ptpWhiteBalanceCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    layout->addWidget(_ptpWhiteBalanceLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpWhiteBalanceCombo, row, 1, 1, 2);
    row++;
    
    // Capture Target
    _ptpCaptureTargetLabel = new QLabel("Save To:");
    _ptpCaptureTargetLabel->setStyleSheet("color: white;");
    _ptpCaptureTargetCombo = new QComboBox();
    _ptpCaptureTargetCombo->addItem("Camera Storage (SD Card)");
    _ptpCaptureTargetCombo->addItem("Computer (USB Transfer)");
    _ptpCaptureTargetCombo->setStyleSheet(
        "QComboBox { background-color: rgba(60, 60, 60, 200); color: white; "
        "border: 2px solid rgba(100, 100, 100, 200); border-radius: 5px; padding: 5px; }"
        "QComboBox QAbstractItemView { background-color: rgba(60, 60, 60, 220); color: white; }"
    );
    layout->addWidget(_ptpCaptureTargetLabel, row, 0, Qt::AlignLeft);
    layout->addWidget(_ptpCaptureTargetCombo, row, 1, 1, 2);
    row++;
}

void CameraControlsPanel::updateControlsVisibility()
{
    bool isPTP = _controller->isPTPCamera();
    
    // Hide resolution and FPS controls for PTP cameras
    if (_resolutionLabel) {
        _resolutionLabel->setVisible(!isPTP);
    }
    if (_resolutionCombo) {
        _resolutionCombo->setVisible(!isPTP);
    }
    if (_fpsLabel) {
        _fpsLabel->setVisible(!isPTP);
    }
    if (_fpsCombo) {
        _fpsCombo->setVisible(!isPTP);
    }
    
    if (_uvcControlsWidget) {
        _uvcControlsWidget->setVisible(!isPTP);
    }
    
    if (_ptpControlsWidget) {
        _ptpControlsWidget->setVisible(isPTP);
        
        // Populate PTP controls if visible
        if (isPTP) {
            // Get capabilities from PTP service
            auto capabilities = _controller->getPTPCapabilities();
            
            // Populate Exposure Mode - filter to Aperture Priority and Manual only for microscopy
            _ptpExposureModeCombo->blockSignals(true);
            _ptpExposureModeCombo->clear();
            if (capabilities.contains("exposuremode")) {
                QStringList modes = capabilities["exposuremode"].toStringList();
                for (const QString& mode : modes) {
                    // Only show Aperture Priority and Manual modes for microscopy
                    if (mode.contains("Aperture", Qt::CaseInsensitive) || 
                        mode.contains("Manual", Qt::CaseInsensitive) ||
                        mode == "A" || mode == "M") {
                        _ptpExposureModeCombo->addItem(mode);
                    }
                }
            }
            _ptpExposureModeCombo->blockSignals(false);
            
            // Populate ISO
            _ptpIsoCombo->blockSignals(true);
            _ptpIsoCombo->clear();
            if (capabilities.contains("iso")) {
                QStringList isoValues = capabilities["iso"].toStringList();
                for (const QString& iso : isoValues) {
                    _ptpIsoCombo->addItem(iso);
                }
            }
            _ptpIsoCombo->blockSignals(false);
            
            // Populate Shutter Speed
            _ptpShutterSpeedCombo->blockSignals(true);
            _ptpShutterSpeedCombo->clear();
            if (capabilities.contains("shutterspeed")) {
                QStringList shutters = capabilities["shutterspeed"].toStringList();
                for (const QString& shutter : shutters) {
                    _ptpShutterSpeedCombo->addItem(shutter);
                }
            }
            _ptpShutterSpeedCombo->blockSignals(false);
            
            // Populate Aperture (hidden for microscopy)
            _ptpApertureCombo->blockSignals(true);
            _ptpApertureCombo->clear();
            if (capabilities.contains("aperture")) {
                QStringList apertures = capabilities["aperture"].toStringList();
                for (const QString& aperture : apertures) {
                    _ptpApertureCombo->addItem(aperture);
                }
            }
            _ptpApertureCombo->blockSignals(false);
            
            // Update shutter speed visibility based on current exposure mode
            QString currentMode = _ptpExposureModeCombo->currentText();
            bool isManualMode = currentMode.contains("Manual", Qt::CaseInsensitive) || currentMode == "M";
            _ptpShutterSpeedLabel->setVisible(isManualMode);
            _ptpShutterSpeedCombo->setVisible(isManualMode);
            
            // Populate White Balance
            _ptpWhiteBalanceCombo->clear();
            if (capabilities.contains("whitebalance")) {
                QStringList wbModes = capabilities["whitebalance"].toStringList();
                for (const QString& wb : wbModes) {
                    _ptpWhiteBalanceCombo->addItem(wb);
                }
            }
            
            // Connect PTP combo signals to handlers (disconnect first to avoid duplicates)
            disconnect(_ptpExposureModeCombo, nullptr, this, nullptr);
            disconnect(_ptpIsoCombo, nullptr, this, nullptr);
            disconnect(_ptpShutterSpeedCombo, nullptr, this, nullptr);
            disconnect(_ptpApertureCombo, nullptr, this, nullptr);
            disconnect(_ptpWhiteBalanceCombo, nullptr, this, nullptr);
            disconnect(_ptpCaptureTargetCombo, nullptr, this, nullptr);
            
            connect(_ptpExposureModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPExposureModeChanged);
            connect(_ptpIsoCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPIsoChanged);
            connect(_ptpShutterSpeedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPShutterSpeedChanged);
            connect(_ptpApertureCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPApertureChanged);
            connect(_ptpWhiteBalanceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPWhiteBalanceChanged);
            connect(_ptpCaptureTargetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    this, &CameraControlsPanel::onPTPCaptureTargetChanged);
        }
    }
}

void CameraControlsPanel::onPTPExposureModeChanged(int index)
{
    if (index < 0) return;
    QString mode = _ptpExposureModeCombo->currentText();
    _controller->setPTPSetting("exposuremode", mode);
    
    // Show/hide shutter speed based on mode (only visible in Manual mode)
    bool isManualMode = mode.contains("Manual", Qt::CaseInsensitive) || mode == "M";
    _ptpShutterSpeedLabel->setVisible(isManualMode);
    _ptpShutterSpeedCombo->setVisible(isManualMode);
}

void CameraControlsPanel::onPTPIsoChanged(int index)
{
    if (index < 0) return;
    QString iso = _ptpIsoCombo->currentText();
    _controller->setPTPSetting("iso", iso);
}

void CameraControlsPanel::onPTPShutterSpeedChanged(int index)
{
    if (index < 0) return;
    QString shutter = _ptpShutterSpeedCombo->currentText();
    _controller->setPTPSetting("shutterspeed", shutter);
}

void CameraControlsPanel::onPTPApertureChanged(int index)
{
    if (index < 0) return;
    QString aperture = _ptpApertureCombo->currentText();
    _controller->setPTPSetting("aperture", aperture);
}

void CameraControlsPanel::onPTPWhiteBalanceChanged(int index)
{
    if (index < 0) return;
    QString wb = _ptpWhiteBalanceCombo->currentText();
    _controller->setPTPSetting("whitebalance", wb);
}

void CameraControlsPanel::onPTPCaptureTargetChanged(int index)
{
    if (index < 0) return;
    // Map UI text to gphoto2 capture target value
    QString target = (index == 0) ? "Memory card" : "Internal RAM";
    _controller->setPTPSetting("capturetarget", target);
}
