# Annotation Rendering Contract

**Module**: Annotation overlay rendering for measurements, object detection, and user markup  
**Owner**: Annotation classes, VideoGraphicsScene overlay layer

## Purpose

Defines the rendering style and techniques for annotations (lines, arrows, text, shapes) that must remain visible and readable on any background, from pure black to pure white and everything in between.

## Core Principle: Outlined Style for Universal Visibility

All annotations use **outlined rendering**: a semi-transparent outline (stroke) behind the main shape/text, ensuring contrast against any background color or texture.

## Rendering Specifications

### 1. Antialiasing (Required)

**All annotations must enable antialiasing** for smooth, professional appearance:

```cpp
void paintAnnotation(QPainter* painter) {
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    // ... draw annotation
}
```

**Rationale**: 
- Eliminates jagged edges on diagonal lines and curves
- Makes text more readable, especially at small sizes
- Professional appearance matching modern UI standards

---

### 2. Line Rendering

**Two-layer approach**: Outline (stroke) + Main line (fill)

```cpp
void drawAnnotationLine(QPainter* painter, const QLineF& line, const QColor& color) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Layer 1: Semi-transparent outline (wider, dark)
    QPen outlinePen(QColor(0, 0, 0, 150));  // Black, 60% opacity
    outlinePen.setWidth(5);  // Wider than main line
    outlinePen.setCapStyle(Qt::RoundCap);
    outlinePen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(outlinePen);
    painter->drawLine(line);
    
    // Layer 2: Main line (bright, fully opaque)
    QPen mainPen(color);  // User-selected color (e.g., yellow, cyan, magenta)
    mainPen.setWidth(2);
    mainPen.setCapStyle(Qt::RoundCap);
    mainPen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(mainPen);
    painter->drawLine(line);
}
```

**Parameters**:
- **Outline color**: Black (`QColor(0, 0, 0, 150)`) or white (`QColor(255, 255, 255, 150)`) depending on adaptive mode
- **Outline opacity**: ~60% (alpha 150) - visible but not overpowering
- **Outline width**: Main line width + 3px (e.g., main=2px → outline=5px)
- **Main line width**: 2-3px (measurement lines), 1-2px (object contours)
- **Main line color**: Bright, high-saturation colors (yellow `#FFFF00`, cyan `#00FFFF`, magenta `#FF00FF`)
- **Cap/Join style**: `Qt::RoundCap` and `Qt::RoundJoin` for smooth corners

**Recommended color palette** (high visibility):
- Measurement lines: Yellow (`#FFFF00`) with black outline
- Object contours: Cyan (`#00FFFF`) with black outline
- User markup: Magenta (`#FF00FF`) with black outline
- Scale bar: White (`#FFFFFF`) with black outline

---

### 3. Text Rendering

**Use QPainterPath for outlined text** (not `drawText()` which can't be outlined):

```cpp
void drawAnnotationText(QPainter* painter, const QPointF& position, 
                        const QString& text, const QFont& font, const QColor& color) {
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);
    
    // Create text path
    QPainterPath textPath;
    textPath.addText(position, font, text);
    
    // Layer 1: Semi-transparent outline (stroke)
    QPen outlinePen(QColor(0, 0, 0, 150));  // Black, 60% opacity
    outlinePen.setWidth(3);  // Thicker outline for text visibility
    outlinePen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(outlinePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(textPath);
    
    // Layer 2: Main text (fill)
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);  // Bright color (yellow, cyan, white)
    painter->drawPath(textPath);
}
```

**Parameters**:
- **Outline width**: 3px (thicker than line outlines for better readability)
- **Outline opacity**: ~60% (alpha 150)
- **Text color**: Same as associated annotation shape (yellow, cyan, magenta, white)
- **Font**: Sans-serif, medium weight, size proportional to view scale (typically 12-16pt)
- **Background**: None - outline provides contrast

**Alternative for simple labels** (when QPainterPath is overkill):

```cpp
void drawSimpleOutlinedText(QPainter* painter, const QPointF& position,
                            const QString& text, const QFont& font, const QColor& color) {
    painter->setFont(font);
    painter->setRenderHint(QPainter::TextAntialiasing);
    
    // Draw outline by drawing text multiple times with offset
    painter->setPen(QColor(0, 0, 0, 150));
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            painter->drawText(position + QPointF(dx, dy), text);
        }
    }
    
    // Draw main text
    painter->setPen(color);
    painter->drawText(position, text);
}
```

**Note**: QPainterPath method is preferred for quality, offset method for performance.

---

### 4. Shape Rendering (Rectangles, Ellipses, Polygons)

**Same two-layer approach** as lines:

```cpp
void drawAnnotationRect(QPainter* painter, const QRectF& rect, const QColor& color) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Layer 1: Outline
    QPen outlinePen(QColor(0, 0, 0, 150));
    outlinePen.setWidth(5);
    outlinePen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(outlinePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
    
    // Layer 2: Main shape
    QPen mainPen(color);
    mainPen.setWidth(2);
    mainPen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(mainPen);
    painter->setBrush(Qt::NoBrush);  // No fill, just stroke
    painter->drawRect(rect);
}
```

**For filled shapes** (detected objects, regions of interest):

```cpp
void drawAnnotationFilledRect(QPainter* painter, const QRectF& rect, const QColor& color) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Outline
    QPen outlinePen(QColor(0, 0, 0, 150));
    outlinePen.setWidth(5);
    painter->setPen(outlinePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
    
    // Main shape with semi-transparent fill
    QPen mainPen(color);
    mainPen.setWidth(2);
    painter->setPen(mainPen);
    painter->setBrush(QColor(color.red(), color.green(), color.blue(), 80));  // 30% opacity fill
    painter->drawRect(rect);
}
```

**Parameters**:
- **Outline**: Same as line rendering (5px width, 60% opacity black)
- **Main border**: 2px width, bright color
- **Fill** (if applicable): Same color as border, 20-30% opacity (alpha 50-80)

---

### 5. Arrow Rendering (for measurement direction, flow indication)

**Arrow = Line + Arrowhead triangle**:

```cpp
void drawAnnotationArrow(QPainter* painter, const QLineF& line, const QColor& color) {
    painter->setRenderHint(QPainter::Antialiasing);
    
    // Draw line portion
    drawAnnotationLine(painter, line, color);
    
    // Calculate arrowhead points
    QPointF p1 = line.p1();
    QPointF p2 = line.p2();
    qreal angle = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
    qreal arrowSize = 12.0;  // Arrow head size in pixels
    
    QPointF arrowP1 = p2 - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                   std::sin(angle + M_PI / 6) * arrowSize);
    QPointF arrowP2 = p2 - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                   std::sin(angle - M_PI / 6) * arrowSize);
    
    QPolygonF arrowHead;
    arrowHead << p2 << arrowP1 << arrowP2;
    
    // Draw arrowhead outline
    QPen outlinePen(QColor(0, 0, 0, 150));
    outlinePen.setWidth(5);
    outlinePen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(outlinePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPolygon(arrowHead);
    
    // Draw arrowhead fill
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(arrowHead);
}
```

---

### 6. Adaptive Outline Color (Advanced)

**For maximum contrast, adapt outline color based on background luminance**:

```cpp
QColor getAdaptiveOutlineColor(const QImage& background, const QPointF& position) {
    // Sample background color at annotation position
    QPoint samplePos = position.toPoint();
    if (samplePos.x() < 0 || samplePos.y() < 0 ||
        samplePos.x() >= background.width() || samplePos.y() >= background.height()) {
        return QColor(0, 0, 0, 150);  // Default to black outline
    }
    
    QRgb pixel = background.pixel(samplePos);
    int luminance = qGray(pixel);  // 0 (black) to 255 (white)
    
    // Use white outline on dark backgrounds, black outline on light backgrounds
    if (luminance < 128) {
        return QColor(255, 255, 255, 150);  // White outline on dark background
    } else {
        return QColor(0, 0, 0, 150);  // Black outline on light background
    }
}
```

**Usage**:
```cpp
QColor outlineColor = getAdaptiveOutlineColor(videoFrame, annotationCenter);
QPen outlinePen(outlineColor);
```

**Note**: Adaptive mode is optional - default black outline works well for most cases.

---

## Annotation Types and Rendering

### Measurement Annotations

**Line measurement** (distance between two points):
- Main line: Yellow (`#FFFF00`), 2px width
- Outline: Black, 60% opacity, 5px width
- Text: Measurement value (e.g., "125.3 µm") at line midpoint, yellow with black outline
- Endpoints: Small circles (5px radius) at each end for visibility

**Area measurement** (polygon or circle):
- Main border: Cyan (`#00FFFF`), 2px width
- Fill: Cyan, 20% opacity (alpha 50)
- Outline: Black, 60% opacity, 5px width
- Text: Area value (e.g., "1520 µm²") at centroid

**Angle measurement** (three-point angle):
- Two lines: Yellow, 2px width with black outline
- Arc: Yellow, 1px width, radius ~30px from vertex
- Text: Angle value (e.g., "45.2°") near arc midpoint

### Object Detection Annotations

**Detected object contour**:
- Main border: Cyan (`#00FFFF`), 1-2px width (thinner than measurement lines)
- Fill: None (transparent) or cyan 10% opacity for highlighting
- Outline: Black, 60% opacity, 4px width
- Label: Object number/ID (e.g., "Obj #12") near centroid, cyan text with black outline

**Bounding box** (optional, for rectangular objects):
- Main border: Magenta (`#FF00FF`), 1px width
- Outline: Black, 60% opacity, 4px width
- No fill

### User Markup Annotations

**Freehand drawing**:
- Main line: User-selected color (default magenta), 2-3px width
- Outline: Black, 60% opacity, 5-6px width
- Cap/Join: `Qt::RoundCap` and `Qt::RoundJoin` for smooth curves

**Text label**:
- Text: User-entered text, white or user-selected color
- Outline: Black, 60% opacity, 3px width
- Font: Sans-serif, 14pt default, user-adjustable

**Arrow pointer**:
- Line + arrowhead: User-selected color, 2px width
- Outline: Black, 60% opacity, 5px width

### Scale Bar Annotation

**Embedded scale bar** (from microscope image):
- Horizontal line: White (`#FFFFFF`), 3px width
- Outline: Black, 60% opacity, 6px width
- Text: Scale value (e.g., "100 µm") centered above line, white with black outline
- End ticks: Vertical lines (10px height) at each end

---

## Performance Considerations

### Rendering Optimization

1. **Cache QPainterPath for text**: Create path once, reuse on repaint
2. **Limit outline draw calls**: Batch annotations with same outline style
3. **Use QGraphicsItem caching**: Enable `ItemCoordinateCache` for static annotations
4. **Disable antialiasing for real-time drawing**: Enable only on final render (image export, video recording)

```cpp
class AnnotationItem : public QGraphicsItem {
public:
    AnnotationItem() {
        // Cache rendering for better performance
        setCacheMode(ItemCoordinateCache);
    }
    
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) override {
        // Only enable antialiasing if not in real-time drawing mode
        if (!_isDrawing) {
            painter->setRenderHint(QPainter::Antialiasing);
        }
        // ... render annotation
    }
    
private:
    bool _isDrawing = false;  // True during interactive drawing
};
```

### Export Rendering

**When exporting annotations to image/video**, always enable:
- Full antialiasing (both `Antialiasing` and `TextAntialiasing`)
- High-quality rendering hints: `HighQualityAntialiasing`, `SmoothPixmapTransform`
- No caching (render fresh for maximum quality)

```cpp
void exportAnnotationsToImage(QPainter* painter) {
    painter->setRenderHints(QPainter::Antialiasing | 
                           QPainter::TextAntialiasing |
                           QPainter::HighQualityAntialiasing |
                           QPainter::SmoothPixmapTransform);
    // ... draw all annotations
}
```

---

## Annotation Data Model

**JSON serialization format** (for metadata embedding):

```json
{
  "annotations": [
    {
      "type": "line_measurement",
      "id": "meas_001",
      "points": [[120.5, 340.2], [450.8, 340.2]],
      "value": 125.3,
      "unit": "µm",
      "color": "#FFFF00",
      "lineWidth": 2,
      "outlineWidth": 5,
      "outlineOpacity": 0.6
    },
    {
      "type": "object_contour",
      "id": "obj_042",
      "contour": [[100, 200], [110, 195], ...],
      "area": 1520.5,
      "circularity": 0.87,
      "color": "#00FFFF",
      "lineWidth": 1,
      "label": "Spore #42"
    },
    {
      "type": "text_label",
      "id": "label_001",
      "position": [300, 150],
      "text": "Interesting feature",
      "font": "Arial",
      "fontSize": 14,
      "color": "#FFFFFF"
    }
  ]
}
```

**Storage**: Embedded in image EXIF/TIFF tags or PNG text chunks (see metadata contract).

---

## Testing Checklist

- [ ] Annotations visible on pure black background (RGB 0,0,0)
- [ ] Annotations visible on pure white background (RGB 255,255,255)
- [ ] Annotations visible on mid-gray background (RGB 128,128,128)
- [ ] Annotations visible on textured/noisy backgrounds (microscopy images)
- [ ] Text remains readable at all zoom levels (50% to 400%)
- [ ] Antialiasing produces smooth lines/curves without jagged edges
- [ ] Outline transparency (~60%) provides contrast without obscuring background details
- [ ] Color palette (yellow, cyan, magenta, white) provides good visibility
- [ ] Arrow heads render correctly at all angles (0° to 360°)
- [ ] Filled shapes have appropriate transparency (20-30%) to show background
- [ ] Performance: 60fps with 50+ annotations on screen (1080p resolution)
- [ ] Exported images have high-quality rendering (no pixelation or artifacts)

---

**Annotation Rendering Status**: ✅ SPECIFIED - Ready for implementation with outlined style for universal visibility
