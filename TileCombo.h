#ifndef TILECOMBO_H
#define TILECOMBO_H

#include "Tile.h"
#include <QGraphicsProxyWidget>
#include <QComboBox>

/**
 * Tile containing an embedded QComboBox for selection lists
 */
class TileCombo : public Tile
{
    Q_OBJECT

public:
    TileCombo(float widthMultiplier = 2.0f, float heightMultiplier = 0.8f, 
              Anchor anchor = Anchor::Center, QGraphicsItem* parent = nullptr);

    QComboBox* comboBox() const { return _comboBox; }

    void addItem(const QString& text, const QVariant& userData = QVariant());
    void setCurrentIndex(int index);
    int currentIndex() const;
    QString currentText() const;

signals:
    void currentTextChanged(const QString& text);
    void currentIndexChanged(int index);

protected:
    void updateGeometry(float baseTileSize, int positionIndex);

private:
    QComboBox* _comboBox;
    QGraphicsProxyWidget* _proxyWidget;
};

#endif // TILECOMBO_H
