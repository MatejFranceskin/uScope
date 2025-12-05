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
- **Transparency**: Semi-transparent background (alpha 0.6-0.9 depending on state)
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

## Base Tile Class API

### Tile Base Class

```cpp
class Tile : public QGraphicsWidget {
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
    Tile(int widthMult, int heightMult, Anchor anchor, QGraphicsItem* parent = nullptr);
    virtual ~Tile() = default;
    
    // Geometry management
    void updateGeometry(int windowWidth, int windowHeight, int baseTileSize);
    int widthMultiplier() const { return _widthMult; }
    int heightMultiplier() const { return _heightMult; }
    Anchor anchor() const { return _anchor; }
    
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
    int _widthMult, _heightMult;
    int _baseTileSize = 100;  // Updated by updateGeometry()
    Anchor _anchor;
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

### TileCombo: Tile with Dropdown

```cpp
class TileCombo : public Tile {
    Q_OBJECT
    
public:
    TileCombo(const QString& iconPath, const QString& label,
              Anchor anchor, QGraphicsItem* parent = nullptr);
    
    void addItem(const QString& text, const QVariant& data = QVariant());
    void setCurrentIndex(int index);
    int currentIndex() const;
    QString currentText() const;
    QVariant currentData() const;
    
signals:
    void currentIndexChanged(int index);
    void currentTextChanged(const QString& text);
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    
private:
    QGraphicsProxyWidget* _comboProxy = nullptr;
    QComboBox* _combo = nullptr;
    QSvgRenderer* _iconRenderer = nullptr;
    QString _label;
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

### TileDialog: Modal Centered Dialog

```cpp
class TileDialog : public Tile {
    Q_OBJECT
    
public:
    /**
     * @brief Construct a modal dialog tile
     * @param widthMult Dialog width (typically 3-5)
     * @param heightMult Dialog height (typically 3-5)
     * @param title Dialog title text
     * @param parent Parent QGraphicsItem
     */
    TileDialog(int widthMult, int heightMult, const QString& title,
               QGraphicsItem* parent = nullptr);
    
    // Content management
    void setContent(QGraphicsWidget* contentWidget);
    void addSubTile(Tile* subTile);  // Add sub-tiles to dialog content area
    
    // Modal behavior
    void show();
    void hide();
    bool isVisible() const;
    
    // Scrolling (if content > dialog area)
    void enableScrolling(bool enable);
    
signals:
    void accepted();
    void rejected();
    void closed();
    
protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void keyPressEvent(QKeyEvent* event) override;  // Esc to close
    
private:
    QString _title;
    QGraphicsWidget* _contentArea = nullptr;
    QGraphicsProxyWidget* _scrollProxy = nullptr;  // For scrolling
    QScrollArea* _scrollArea = nullptr;
    QList<Tile*> _subTiles;
    bool _scrollingEnabled = false;
    
    void updateContentLayout();
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
    
    // Tile collections
    QList<Tile*> _leftTiles;
    QList<Tile*> _rightTiles;
    TileDialog* _activeDialog = nullptr;
    QGraphicsRectItem* _dialogOverlay = nullptr;  // Dimming overlay
    
    // Layout management
    void updateTileLayout();
    int calculateBaseTileSize() const;
    void positionTiles(QList<Tile*>& tiles, Tile::Anchor anchor);
    
    // Constants
    static constexpr int BASE_TILE_DIVISOR = 12;
    static constexpr int TILE_MARGIN = 15;
    static constexpr int TILE_SPACING = 10;
};
```

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
    switch (_state) {
    case Idle:
        return QColor(100, 100, 100, 180);  // Gray, 70% opaque
    case Hover:
        return QColor(120, 120, 150, 200);  // Light blue, 78% opaque
    case Active:
        return QColor(80, 150, 80, 220);    // Green, 86% opaque
    case Disabled:
        return QColor(80, 80, 80, 150);     // Dark gray, 59% opaque
    default:
        return QColor(100, 100, 100, 180);
    }
}

void Tile::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing);  // Smooth rounded corners
    QRectF rect = boundingRect();
    int radius = cornerRadius();  // baseTileSize / 8
    
    // Draw rounded rectangle background
    painter->setBrush(getStateColor());
    if (_borderWidth > 0) {
        painter->setPen(QPen(_borderColor, _borderWidth));
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(rect, radius, radius);
}
```

## Usage Examples

### Creating Main Window Tiles

```cpp
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Setup view and scene
    _scene = new VideoGraphicsScene(this);
    _view = new QGraphicsView(_scene, this);
    setCentralWidget(_view);
    
    // Left-side tiles
    auto* cameraSelectTile = new TileCombo(":/images/camera.svg", "Camera", Tile::Left);
    cameraSelectTile->addItem("USB Camera 1", "/dev/video0");
    cameraSelectTile->addItem("USB Camera 2", "/dev/video1");
    connect(cameraSelectTile, &TileCombo::currentIndexChanged, 
            this, &MainWindow::onCameraChanged);
    _leftTiles.append(cameraSelectTile);
    _scene->addItem(cameraSelectTile);
    
    auto* settingsTile = new TileButton(":/images/settings.svg", "Settings", Tile::Left);
    connect(settingsTile, &TileButton::clicked, this, &MainWindow::showSettingsDialog);
    _leftTiles.append(settingsTile);
    _scene->addItem(settingsTile);
    
    // Right-side tiles
    auto* captureTile = new TileButton(":/images/camera.svg", "Capture", Tile::Right);
    connect(captureTile, &TileButton::clicked, this, &MainWindow::captureImage);
    _rightTiles.append(captureTile);
    _scene->addItem(captureTile);
    
    auto* exposureSlider = new TileSlider(":/images/settings.svg", "Exposure", 
                                          10, 1000, 100, Tile::Right);
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
    auto* dialog = new TileDialog(4, 5, "Calibration", nullptr);
    
    // Add sub-tiles to dialog content
    auto* objectiveLabel = new TileLabel(":/images/microscope.svg", "Objective:", Tile::Center);
    dialog->addSubTile(objectiveLabel);
    
    auto* objectiveCombo = new TileCombo("", "", Tile::Center);
    objectiveCombo->addItem("4x");
    objectiveCombo->addItem("10x");
    objectiveCombo->addItem("40x");
    dialog->addSubTile(objectiveCombo);
    
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
