#include "TileLabel.h"
#include <QStyleOptionGraphicsItem>
#include <QFont>

TileLabel::TileLabel(const QString& svgPath, const QString& text, float widthMultiplier, 
                     float heightMultiplier, Anchor anchor, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, anchor, parent)
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
    // Draw base tile background
    Tile::paint(painter, option, widget);
    
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    
    QRectF rect = boundingRect();
    
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
        font.setPixelSize(static_cast<int>(rect.height() * 0.12f));  // 12% of tile height
        painter->setFont(font);
        
        QRectF textRect(rect.left(), rect.top() + iconHeight, rect.width(), textHeight);
        painter->drawText(textRect, Qt::AlignCenter, _text);
    }
}
