#ifndef TILESLIDER_H
#define TILESLIDER_H

#include "Tile.h"
#include <QGraphicsProxyWidget>
#include <QSlider>
#include <QLabel>

/**
 * Tile containing an embedded QSlider for numeric value selection
 */
class TileSlider : public Tile
{
    Q_OBJECT

public:
    TileSlider(Qt::Orientation orientation = Qt::Horizontal, 
               float widthMultiplier = 2.0f, float heightMultiplier = 0.8f,
               Anchor anchor = Anchor::Center, int gridX = 0, int gridY = 0, QGraphicsItem* parent = nullptr);

    QSlider* slider() const { return _slider; }

    void setRange(int min, int max);
    void setValue(int value);
    int value() const;
    
    void setLabel(const QString& label);

signals:
    void valueChanged(int value);

protected:
    void updateGeometry(float baseTileSize, float sceneWidth);

private:
    QSlider* _slider;
    QLabel* _label;
    QGraphicsProxyWidget* _proxyWidget;
    Qt::Orientation _orientation;
};

#endif // TILESLIDER_H
