#ifndef ZOOMTILE_H
#define ZOOMTILE_H

#include "../TileButton.h"

class ZoomController;
class TileButton;

/**
 * @brief Zoom tile displaying current zoom factor with controls panel
 * 
 * The zoom tile shows the current zoom factor (e.g., "2.3x", "Fit W", "100%")
 * and can be clicked to toggle a panel with zoom control buttons.
 * 
 * Panel buttons:
 * - "1:1" for OneToOne (100%) zoom
 * - "Fit W" for FitWidth zoom
 * - "Fit H" for FitHeight zoom
 */
class ZoomTile : public TileButton
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new ZoomTile
     * @param zoomController Zoom controller to monitor and control
     * @param parent Parent QGraphicsItem
     */
    explicit ZoomTile(ZoomController* zoomController, QGraphicsItem* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ZoomTile() override;
    
    /**
     * @brief Update tile layout based on base tile size and scene width
     * @param baseTileSize Base size for tile calculations
     * @param sceneWidth Width of the scene
     */
    void updateGeometry(float baseTileSize, float sceneWidth);

protected:
    /**
     * @brief Paint the tile with current zoom display
     * @param painter Painter to use
     * @param option Style options
     * @param widget Widget being painted on
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private slots:
    /**
     * @brief Handle zoom tile click to toggle panel
     */
    void onZoomTileClicked();
    
    /**
     * @brief Handle zoom state changes to update display
     */
    void onZoomStateChanged();
    
    /**
     * @brief Handle OneToOne button click
     */
    void onOneToOneClicked();
    
    /**
     * @brief Handle FitWidth button click
     */
    void onFitWidthClicked();
    
    /**
     * @brief Handle FitHeight button click
     */
    void onFitHeightClicked();

private:
    /**
     * @brief Show the zoom controls panel
     */
    void showPanel();
    
    /**
     * @brief Hide the zoom controls panel
     */
    void hidePanel();
    
    /**
     * @brief Create panel buttons
     */
    void createPanelButtons();

    ZoomController* _zoomController;  ///< Zoom controller
    QString _displayText;              ///< Current display text
    
    // Panel control buttons
    TileButton* _oneToOneButton;       ///< 1:1 zoom button
    TileButton* _fitWidthButton;       ///< Fit width button
    TileButton* _fitHeightButton;      ///< Fit height button
    bool _panelVisible;                ///< Whether panel is visible
};

#endif // ZOOMTILE_H
