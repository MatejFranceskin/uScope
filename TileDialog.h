#ifndef TILEDIALOG_H
#define TILEDIALOG_H

#include "Tile.h"
#include <QGraphicsRectItem>
#include <QGraphicsProxyWidget>
#include <QScrollArea>
#include <QWidget>

// Forward declaration
class TileDialog;

/**
 * Clickable overlay that closes dialog when clicked
 */
class DimOverlay : public QGraphicsRectItem
{
public:
    explicit DimOverlay(TileDialog* dialog);
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    
private:
    TileDialog* _dialog;
};

/**
 * Modal dialog tile with semi-transparent overlay
 * Centered horizontally, positioned vertically via grid Y coordinate
 * Displayed with dimmed background overlay
 */
class TileDialog : public Tile
{
    Q_OBJECT

public:
    TileDialog(float widthMultiplier = 4.0f, float heightMultiplier = 6.0f,
               int gridY = 0, QGraphicsItem* parent = nullptr);
    
    virtual ~TileDialog();

    void show(float baseTileSize, float sceneWidth);
    void hide();
    bool isVisible() const;
    
    // Content widget access - subclasses set content during construction
    void setContentWidget(QWidget* content);
    QWidget* contentWidget() const;
    
    /**
     * @brief Update geometry of dialog and child tiles
     */
    void updateGeometry(float baseTileSize, float sceneWidth, int fontSize = 0) override;

signals:
    void accepted();
    void rejected();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    DimOverlay* _dimOverlay;
    QGraphicsProxyWidget* _proxyWidget;
    QScrollArea* _scrollArea;
    QWidget* _contentWidget;
    
    void setupScrollArea();
    void updateProxyWidgetGeometry();
    void updateContentWidgetStyle();
};

#endif // TILEDIALOG_H
