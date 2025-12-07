#ifndef TILELABEL_H
#define TILELABEL_H

#include "Tile.h"
#include <QSvgRenderer>
#include <QString>
#include <QSharedPointer>

/**
 * Tile displaying an SVG icon with text label below
 */
class TileLabel : public Tile
{
    Q_OBJECT

public:
    TileLabel(const QString& svgPath, const QString& text, float widthMultiplier = 1.0f, 
              float heightMultiplier = 1.0f, Anchor anchor = Anchor::Center, 
              int gridX = 0, int gridY = 0, QGraphicsItem* parent = nullptr);

    void setText(const QString& text);
    QString text() const { return _text; }

    void setSvgPath(const QString& svgPath);

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    QSharedPointer<QSvgRenderer> _svgRenderer;
    QString _text;
};

#endif // TILELABEL_H
