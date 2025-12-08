# Tile UI Architecture Contract

**Module**: Resolution-independent tile-based overlay UI system  
**Owner**: MainWindow, Tile class hierarchy

## Purpose

Defines the custom tile-based UI architecture that provides consistent, resolution-independent appearance across all window sizes and DPIs. Video feed displays as background with transparent tile widgets overlayed on top.

## Core Principles

### 1. Proportional Scaling
- **Base Tile Size**: `windowHeight / 12` (divisor = 12, configurable constant)
- **Tile Dimensions**: Width = `widthMultiplier × baseTileSize`, Height = `heightMultiplier × baseTileSize`
- **Font Scaling**: `fontSize = baseTileSize / 4` (ensures text fits within tiles)
- **Icon Scaling**: SVG icons scaled to `baseTileSize × 0.6` (60% of tile size)

### 2. Tile Properties
- **Shape**: Square (1×1) or rectangle (N×M multipliers, e.g., 2×1, 3×2) with rounded corners
- **Corner Radius**: `baseTileSize / 8` (proportional scaling, typically 8-15px depending on window size)
- **Transparency**: Semi-transparent background (alpha 150-220 depending on state: Idle=200, Hover=220, Active=220, Disabled=150)
- **Background Color**: State-dependent (Idle, Hover, Active, Disabled)
- **Border**: Optional 1-2px border for emphasis (rounded to match corners)
- **Content**: Icon (top/center) + Text (bottom/center) typical layout

### 3. Anchoring System
- **Left Anchor**: `x = MARGIN` (typically 10-20px from left edge)
- **Right Anchor**: `x = windowWidth - tileWidth - MARGIN`
- **Center Anchor**: `x = (windowWidth - tileWidth) / 2`
- **Vertical Stacking**: Tiles anchored to same side stack vertically with `TILE_SPACING` gap

### 4. Layering (Z-Order)
1. **Background Layer** (z=0): Video feed (QGraphicsPixmapItem or QGraphicsVideoItem)
2. **Tile Layer** (z=10): Interactive tiles (left/right anchored)
3. **Dialog Overlay** (z=20): Semi-transparent black dimming (when modal dialog active)
4. **Modal Dialog Layer** (z=30): Centered dialog tiles

### 5. Rendering Architecture (CRITICAL)

**Base Class**: `QGraphicsObject` (NOT QGraphicsWidget)
- QGraphicsWidget has default widget rendering that draws opaque backgrounds/frames
- This causes visual artifacts (black corners outside rounded rectangles)
- QGraphicsObject is lighter weight and has no default rendering
- Must implement `boundingRect()` and `paint()` for custom drawing

**Scene Rendering Strategy**:
- Video background rendered in `VideoGraphicsScene::drawBackground()`
- Tiles rendered as QGraphicsObject items on top
- Scene invalidation: `invalidate(sceneRect(), BackgroundLayer)` updates only video
- **CRITICAL**: Must use `FullViewportUpdate` mode for flicker-free rendering

**Viewport Update Mode**:
```cpp
_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
```
- **FullViewportUpdate**: Renders entire scene to off-screen buffer, then swaps atomically (double buffering)
- **MinimalViewportUpdate**: Attempts minimal region updates but causes tiles to repaint on every video frame
- Video updates at 30-60 fps - MinimalViewportUpdate triggers item repaints unnecessarily
- FullViewportUpdate ensures tiles only repaint when their state actually changes

**Item Caching**:
```cpp
setCacheMode(ItemCoordinateCache);
setFlag(ItemClipsToShape, true);
```
- ItemCoordinateCache: Caches item rendering in item's coordinate system
- ItemClipsToShape: Ensures clicks/hover only within rounded rectangle shape
- Cache persists across video frame updates when using FullViewportUpdate

**Paint Implementation**:
```cpp
void Tile::paint(QPainter* painter, ...) {
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(boundingRect(), radius, radius);
    painter->fillPath(roundedPath, backgroundColor);
    // Draw content...
}

QPainterPath Tile::shape() const {
    QPainterPath path;
    path.addRoundedRect(boundingRect(), radius, radius);
    return path;  // Used for hit testing and clipping
}
```

**Key Insights**:
1. Video updates should NOT trigger tile repaints - tiles are independent items
2. FullViewportUpdate provides proper double buffering for smooth rendering
3. QGraphicsObject avoids unwanted default widget rendering
4. Semi-transparent tiles work correctly with proper viewport mode (no alpha blending flicker)

## Base Tile Class API

### Tile Base Class

```cpp
class Tile : public QGraphicsObject {  // NOT QGraphicsWidget - see Rendering Architecture
    Q_OBJECT
    Q_PROPERTY(TileState state READ state WRITE setState NOTIFY stateChanged)
    
public:
    enum Anchor { Left, Right, Center };
    enum TileState { Idle, Hover, Active, Disabled };
    
    /**
     * @brief Construct a tile
     * @param widthMult Tile width in multiples of base tile size (1, 2, 3, ...)
     * @param heightMult Tile height in multiples of base tile size (1, 2, 3, ...)
     * @param anchor Positioning anchor (Left, Right, Center)
     * @param parent Parent QGraphicsItem
     */
    Tile(float widthMult, float heightMult, Anchor anchor, int stackPosition, int maxStack, QGraphicsItem* parent = nullptr);
    virtual ~Tile() = default;
    
    // Geometry management (QGraphicsObject requires boundingRect)
    QRectF boundingRect() const override { return QRectF(0, 0, _width, _height); }
    
    // CRITICAL: Must be virtual for TileDialog to properly override and update dim overlay
    virtual void updateGeometry(int windowWidth, int windowHeight, int baseTileSize);
    
    float widthMultiplier() const { return _widthMult; }
    float heightMultiplier() const { return _heightMult; }
    Anchor anchor() const { return _anchor; }
    
    // Shape for hit testing and clipping
    QPainterPath shape() const override;
    
    // State management
    TileState state() const { return _state; }
    void setState(TileState state);
    
    // Appearance customization
    void setBackgroundColor(const QColor& color);
    void setTransparency(qreal alpha);  // 0.0 (transparent) to 1.0 (opaque)
    void setBorderColor(const QColor& color);
    void setBorderWidth(int pixels);
    int cornerRadius() const { return _baseTileSize / 8; }  // Proportional corner rounding
    
signals:
    void clicked();
    void stateChanged(TileState newState);
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    
    // Helper for derived classes to get scaled sizes
    int baseTileSize() const { return _baseTileSize; }
    int scaledFontSize() const { return _baseTileSize / 4; }
    QSizeF scaledIconSize() const { return QSizeF(_baseTileSize * 0.6, _baseTileSize * 0.6); }
    
private:
    float _widthMult, _heightMult;
    int _baseTileSize = 100;  // Updated by updateGeometry()
    float _width = 100.0f, _height = 100.0f;  // Required for QGraphicsObject
    Anchor _anchor;
    int _stackPosition, _maxStack;
    TileState _state = Idle;
    QColor _backgroundColor;
    qreal _transparency = 0.7;
    QColor _borderColor;
    int _borderWidth = 0;
    
    QColor getStateColor() const;  // Returns color based on _state
};
```

### TileLabel: Icon + Text Tile

```cpp
class TileLabel : public Tile {
    Q_OBJECT
    
public:
    TileLabel(const QString& iconPath, const QString& text, 
              Anchor anchor, QGraphicsItem* parent = nullptr);
    
    void setIcon(const QString& svgPath);
    void setText(const QString& text);
    QString text() const { return _text; }
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    
private:
    QSvgRenderer* _iconRenderer = nullptr;
    QString _text;
};
```

### TileButton: Clickable Action Tile

```cpp
class TileButton : public TileLabel {
    Q_OBJECT
    
public:
    TileButton(const QString& iconPath, const QString& text,
               Anchor anchor, QGraphicsItem* parent = nullptr);
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return state() != Disabled; }
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
};
```

### TileSlider: Tile with Slider Control

```cpp
class TileSlider : public Tile {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    
public:
    TileSlider(const QString& iconPath, const QString& label,
               int min, int max, int value,
               Anchor anchor, QGraphicsItem* parent = nullptr);
    
    int value() const;
    void setValue(int value);
    void setRange(int min, int max);
    
signals:
    void valueChanged(int value);
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    
private:
    QGraphicsProxyWidget* _sliderProxy = nullptr;
    QSlider* _slider = nullptr;
    QSvgRenderer* _iconRenderer = nullptr;
    QString _label;
    int _min, _max;
};
```

### TileDialog: Modal Centered Dialog with Qt Controls

**CRITICAL Implementation Notes**:
1. **Virtual updateGeometry**: Base Tile class MUST declare `updateGeometry()` as virtual for proper polymorphic behavior
2. **Tile List Integration**: Dialogs MUST be added to MainWindow's `_centerTiles` list to receive resize updates
3. **Clickable Dim Overlay**: Custom DimOverlay class handles mouse clicks to close dialog when clicking outside
4. **Resize Synchronization**: Override `updateGeometry()` to update both dialog and dim overlay on window resize
5. **Tile Size Calculation**: Use consistent baseTileSize from MainWindow (height/8), passed to show() method
6. **Qt Widget Integration**: Contains QGraphicsProxyWidget for embedding standard Qt controls (QListWidget, QPushButton, QLineEdit, etc.)
7. **Scrollable Content**: Content area is scrollable when content exceeds dialog size via QScrollArea embedded in proxy widget
8. **Centralized Font Sizing**: Font size calculated once in MainWindow::calculateFontSize() (baseTileSize * 0.12) and passed through updateGeometry() to ensure consistent, scalable fonts across all tiles and dialog controls

```cpp
// Clickable overlay that closes dialog when clicked outside
class DimOverlay : public QGraphicsRectItem
{
public:
    explicit DimOverlay(TileDialog* dialog);
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;  // Closes dialog on click
    
private:
    TileDialog* _dialog;
};

class TileDialog : public Tile {
    Q_OBJECT
    
public:
    /**
     * @brief Construct a modal dialog tile with Qt widget content
     * @param widthMultiplier Dialog width in tiles (typically 4-8)
     * @param heightMultiplier Dialog height in tiles (typically 3-6)
     * @param gridY Vertical grid position (Y coordinate in tile units, 0=top)
     * @param parent Parent QGraphicsItem
     * 
     * Dialog is centered horizontally (Anchor::Center) and positioned 
     * vertically at gridY * baseTileSize from top of scene.
     * 
     * Contains a QGraphicsProxyWidget that hosts a QScrollArea with content widget.
     * Content can be larger than dialog viewport - scroll bars appear automatically.
     */
    TileDialog(float widthMultiplier = 4.0f, float heightMultiplier = 6.0f,
               int gridY = 0, QGraphicsItem* parent = nullptr);
    
    virtual ~TileDialog();
    
    // Modal behavior
    void show(float baseTileSize, float sceneWidth);  // Pass consistent sizing from MainWindow
    void hide();
    bool isVisible() const;
    
    // CRITICAL: Override to update dim overlay and resize proxy widget when window resizes
    // fontSize parameter from MainWindow ensures consistent font scaling
    void updateGeometry(float baseTileSize, float sceneWidth, int fontSize = 0) override;
    
    // Content widget access - subclasses set content during construction
    void setContentWidget(QWidget* content);
    QWidget* contentWidget() const;
    
signals:
    void accepted();
    void rejected();
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void keyPressEvent(QKeyEvent* event) override;  // Esc to close -> emit rejected()
    
private:
    DimOverlay* _dimOverlay = nullptr;               // Clickable semi-transparent overlay
    QGraphicsProxyWidget* _proxyWidget = nullptr;   // Hosts Qt widget content
    QScrollArea* _scrollArea = nullptr;             // Provides scrolling for oversized content
    QWidget* _contentWidget = nullptr;              // Actual dialog content set by subclass
    
    void setupScrollArea();
    void updateProxyWidgetGeometry();
    void updateContentWidgetStyle();  // Apply centralized fontSize to all child widgets
};
```

**Implementation Pattern for Subclassed Dialogs**:
```cpp
class CameraSettingsDialog : public TileDialog {
public:
    CameraSettingsDialog(CameraController* controller, QGraphicsItem* parent = nullptr)
        : TileDialog(6.0f, 8.0f, 1, parent)  // 6×8 tiles, Y=1 from top
        , _controller(controller)
    {
        setupUI();
    }
    
private:
    void setupUI() {
        // Create content widget with standard Qt controls
        QWidget* content = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(content);
        
        // Camera selection with QListWidget - instant camera switching
        QLabel* cameraLabel = new QLabel("Select Camera:");
        _cameraList = new QListWidget();
        _cameraList->addItems(_controller->availableCameras());
        
        // Assemble layout (no OK/Cancel buttons - changes apply instantly)
        layout->addWidget(cameraLabel);
        layout->addWidget(_cameraList);
        
        // Set as dialog content - scroll bars appear automatically if needed
        // Fonts will be set automatically by TileDialog based on MainWindow::calculateFontSize()
        setContentWidget(content);
        
        // Connect signals - camera switches instantly on selection
        connect(_cameraList, &QListWidget::currentRowChanged,
                this, &CameraSettingsDialog::onCameraChanged);
        // Dialog closes via ESC key or clicking outside (on dim overlay)
    }
    
    CameraController* _controller;
    QListWidget* _cameraList;
};
```

## MainWindow Integration

### MainWindow API

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    
private slots:
    void onFrameReady(const QVideoFrame& frame);
    void onTileClicked();
    void showDialog(TileDialog* dialog);
    void hideDialog(TileDialog* dialog);
    
private:
    QGraphicsView* _view = nullptr;
    VideoGraphicsScene* _scene = nullptr;
    
    // Tile collections - CRITICAL: Dialogs MUST be in _centerTiles to receive resize updates
    QList<Tile*> _leftTiles;
    QList<Tile*> _rightTiles;
    QList<Tile*> _centerTiles;  // Contains centered elements including TileDialogs
    
    // Layout management
    void updateTileLayout();
    float calculateBaseTileSize() const { return height() / 8.0f; }  // Consistent calculation
    int calculateFontSize() const { return static_cast<int>(calculateBaseTileSize() * 0.12f); }  // Centralized font sizing
    
    // Constants
    static constexpr int BASE_TILE_DIVISOR = 8;  // height / 8 for base tile size
    static constexpr int TILE_MARGIN = 15;
    static constexpr int TILE_SPACING = 10;
};
```

### Dialog Integration Pattern

```cpp
void MainWindow::createTiles() {
    // Create dialog and ADD TO _centerTiles for resize updates
    _cameraPanel = new CameraControlsPanel(_cameraController);
    _scene->addItem(_cameraPanel);
    _centerTiles.append(_cameraPanel);  // CRITICAL: Must be in tile list
    _cameraPanel->hide();
    
    connect(_cameraPanel, &CameraControlsPanel::accepted, ...);
    connect(_cameraPanel, &CameraControlsPanel::rejected, ...);
}

void MainWindow::onCameraButtonClicked() {
    if (_cameraPanel->isVisible()) {
        _cameraPanel->hide();
    } else {
        // Pass consistent tile sizing from MainWindow
        float baseTileSize = calculateBaseTileSize();
        QRectF sceneRect = _scene->sceneRect();
        _cameraPanel->show(baseTileSize, sceneRect.width());
    }
}

### Layout Algorithm

```cpp
void MainWindow::updateTileLayout() {
    int baseTileSize = calculateBaseTileSize();
    int windowWidth = _view->width();
    int windowHeight = _view->height();
    
    // Update all tiles with new geometry
    for (Tile* tile : _leftTiles) {
        tile->updateGeometry(windowWidth, windowHeight, baseTileSize);
    }
    for (Tile* tile : _rightTiles) {
        tile->updateGeometry(windowWidth, windowHeight, baseTileSize);
    }
    
    // Position tiles vertically
    positionTiles(_leftTiles, Tile::Left);
    positionTiles(_rightTiles, Tile::Right);
    
    // Update dialog if visible
    if (_activeDialog) {
        _activeDialog->updateGeometry(windowWidth, windowHeight, baseTileSize);
        // Center dialog
        qreal x = (windowWidth - _activeDialog->boundingRect().width()) / 2;
        qreal y = (windowHeight - _activeDialog->boundingRect().height()) / 2;
        _activeDialog->setPos(x, y);
    }
}

void MainWindow::positionTiles(QList<Tile*>& tiles, Tile::Anchor anchor) {
    int yOffset = TILE_MARGIN;
    
    for (Tile* tile : tiles) {
        QRectF bounds = tile->boundingRect();
        qreal x = 0;
        
        switch (anchor) {
        case Tile::Left:
            x = TILE_MARGIN;
            break;
        case Tile::Right:
            x = _view->width() - bounds.width() - TILE_MARGIN;
            break;
        case Tile::Center:
            x = (_view->width() - bounds.width()) / 2;
            break;
        }
        
        tile->setPos(x, yOffset);
        yOffset += bounds.height() + TILE_SPACING;
    }
}
```

## State-Based Color Scheme

```cpp
QColor Tile::getStateColor() const {
    // Semi-transparent backgrounds for video overlay effect
    switch (_state) {
    case Idle:
        return QColor(40, 40, 40, 200);   // Dark semi-transparent
    case Hover:
        return QColor(60, 60, 60, 220);   // Lighter on hover
    case Active:
        return QColor(0, 120, 215, 220);  // Qt blue for active
    case Disabled:
        return QColor(30, 30, 30, 150);   // Darker, more transparent
    default:
        return QColor(40, 40, 40, 200);
    }
}

void Tile::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Create rounded rectangle path
    int radius = _baseTileSize / 8;
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(boundingRect(), radius, radius);
    
    // Fill with semi-transparent state color
    painter->fillPath(roundedPath, getStateColor());
    
    // Optional border
    if (_borderWidth > 0) {
        painter->setPen(QPen(_borderColor, _borderWidth));
        painter->drawPath(roundedPath);
    }
}

QPainterPath Tile::shape() const {
    // Returns rounded rectangle for accurate hit testing
    int radius = _baseTileSize / 8;
    QPainterPath path;
    path.addRoundedRect(boundingRect(), radius, radius);
    return path;
}
```

## Usage Examples

### Creating Main Window Tiles

```cpp
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Setup scene and view
    _scene = new VideoGraphicsScene(this);
    _view = new QGraphicsView(_scene, this);
    setCentralWidget(_view);
    
    // CRITICAL: Configure viewport for flicker-free rendering
    _view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    _view->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing, true);
    
    // Left-side tiles for main controls
    auto* cameraButton = new TileButton(":/images/camera.svg", "Camera", 
                                        1.0f, 1.0f, Tile::Left, 0, 0);
    connect(cameraButton, &TileButton::clicked, 
            this, &MainWindow::showCameraDialog);
    _leftTiles.append(cameraButton);
    _scene->addItem(cameraButton);
    
    auto* settingsTile = new TileButton(":/images/settings.svg", "Settings", 
                                        Tile::Left, 1, 2);
    connect(settingsTile, &TileButton::clicked, this, &MainWindow::showSettingsDialog);
    _leftTiles.append(settingsTile);
    _scene->addItem(settingsTile);
    
    // Right-side tiles
    auto* captureTile = new TileButton(":/images/camera.svg", "Capture", 
                                       Tile::Right, 0, 2);
    connect(captureTile, &TileButton::clicked, this, &MainWindow::captureImage);
    _rightTiles.append(captureTile);
    _scene->addItem(captureTile);
    
    auto* exposureSlider = new TileSlider(":/images/settings.svg", "Exposure", 
                                          10, 1000, 100, Tile::Right, 1, 2);
    connect(exposureSlider, &TileSlider::valueChanged, 
            _cameraController, &CameraController::setExposure);
    _rightTiles.append(exposureSlider);
    _scene->addItem(exposureSlider);
    
    // Initial layout
    updateTileLayout();
}
```

### Creating Modal Dialog

```cpp
void MainWindow::showCalibrationDialog() {
    auto* dialog = new CalibrationDialog(_calibrationController, nullptr);
    
    connect(dialog, &TileDialog::accepted, this, &MainWindow::onCalibrationAccepted);
    connect(dialog, &TileDialog::rejected, this, &MainWindow::onCalibrationRejected);
    
    showDialog(dialog);
}

void MainWindow::showDialog(TileDialog* dialog) {
    // Create dimming overlay
    _dialogOverlay = new QGraphicsRectItem(0, 0, _view->width(), _view->height());
    _dialogOverlay->setBrush(QColor(0, 0, 0, 180));  // 70% black
    _dialogOverlay->setZValue(20);
    _scene->addItem(_dialogOverlay);
    
    // Show dialog on top
    dialog->setZValue(30);
    _scene->addItem(dialog);
    _activeDialog = dialog;
    
    updateTileLayout();  // Centers dialog
    dialog->show();
}
```

## Performance Considerations

1. **Resize Throttling**: Update tile layout only on resize events, not every frame
2. **Icon Caching**: Cache scaled SVG icons at current resolution, regenerate only on resize
3. **Dirty Region Updates**: Use `QGraphicsItem::update()` for individual tiles instead of `scene->update()`
4. **Video Background**: Use `QGraphicsVideoItem` or hardware-accelerated `QGraphicsPixmapItem`
5. **Z-Order Optimization**: Minimize layer count to reduce overdraw

## Testing Checklist

- [ ] Tiles scale proportionally on window resize (test 800×600 to 3840×2160)
- [ ] Text remains readable at all window sizes (min 10pt, max 24pt)
- [ ] Icons scale without pixelation (use SVG, not raster)
- [ ] Touch targets meet 44x44pt minimum on high-DPI displays
- [ ] Modal dialogs center correctly on all resolutions
- [ ] Video background visible through transparent tiles
- [ ] State transitions (Idle→Hover→Active) visually distinct
- [ ] Keyboard navigation works (Tab, Enter, Esc)
- [ ] High-DPI displays (devicePixelRatio > 1) render correctly
- [ ] Tile anchoring (Left/Right/Center) works on ultra-wide displays
- [ ] Rounded corners render smoothly with antialiasing (no jagged edges)
- [ ] Corner radius scales proportionally with tile size (radius = baseTileSize / 8)

---

**Tile UI Architecture Status**: ✅ SPECIFIED - Ready for implementation with complete API and examples
