#include "TileLabel.h"
#include <QStyleOptionGraphicsItem>
#include <QFont>

TileLabel::TileLabel(const QString& svgPath, const QString& text, float widthMultiplier, 
                     float heightMultiplier, Anchor anchor, int gridX, int gridY, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, anchor, gridX, gridY, parent)
    , _text(text)
{
    if (!svgPath.isEmpty()) {
        _svgRenderer = QSharedPointer<QSvgRenderer>(new QSvgRenderer(svgPath));
    }
}

void TileLabel::setText(const QString& text)
{
    if (_text != text) {
        _text = text;
        update();
    }
}

void TileLabel::setSvgPath(const QString& svgPath)
{
    if (!svgPath.isEmpty()) {
        _svgRenderer = QSharedPointer<QSvgRenderer>(new QSvgRenderer(svgPath));
        update();
    }
}

void TileLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    QRectF rect = boundingRect();
    float radius = Tile::cornerRadius(_currentBaseTileSize);
    
    // Create rounded rectangle path  
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(rect, radius, radius);
    
    // Fill ONLY the rounded rectangle area
    QColor bgColor;
    switch (_state) {
        case TileState::Idle:
            bgColor = QColor(40, 40, 40, 200);
            break;
        case TileState::Hover:
            bgColor = QColor(60, 60, 60, 220);
            break;
        case TileState::Active:
            bgColor = QColor(0, 120, 215, 220);
            break;
        case TileState::Disabled:
            bgColor = QColor(30, 30, 30, 150);
            break;
    }
    
    painter->fillPath(roundedPath, bgColor);
    
    // Clip to rounded rectangle for icon and text
    painter->setClipPath(roundedPath);
    
    // Calculate layout: icon takes 60% height, text takes 40%
    float iconHeight = rect.height() * 0.6f;
    float textHeight = rect.height() * 0.4f;
    
    // Draw SVG icon centered in top 60%
    if (_svgRenderer && _svgRenderer->isValid()) {
        QRectF iconRect(rect.left() + rect.width() * 0.1f,
                        rect.top() + rect.height() * 0.05f,
                        rect.width() * 0.8f,
                        iconHeight);
        _svgRenderer->render(painter, iconRect);
    }
    
    // Draw text label centered in bottom 40%
    if (!_text.isEmpty()) {
        painter->setPen(Qt::white);
        QFont font = painter->font();
        font.setPixelSize(static_cast<int>(rect.height() * 0.12f * 1.3f));
        font.setBold(true);
        painter->setFont(font);
        
        QRectF textRect(rect.left(), rect.top() + iconHeight, rect.width(), textHeight);
        painter->drawText(textRect, Qt::AlignCenter, _text);
    }
    
    painter->restore();
}
