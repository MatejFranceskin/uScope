#ifndef ZOOMABLEGRAPHICSVIEW_H
#define ZOOMABLEGRAPHICSVIEW_H

#include <QGraphicsView>

class ZoomController;

/**
 * @brief QGraphicsView subclass with zoom and pan support
 * 
 * This view handles mouse wheel events for zooming and mouse drag events
 * for panning when zoomed. It delegates zoom/pan logic to ZoomController.
 */
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new ZoomableGraphicsView
     * @param scene The graphics scene to display
     * @param parent Parent widget
     */
    explicit ZoomableGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr);
    
    /**
     * @brief Set the zoom controller
     * @param controller Zoom controller to use for zoom/pan operations
     */
    void setZoomController(ZoomController* controller);
    
    /**
     * @brief Get the zoom controller
     * @return Pointer to the zoom controller
     */
    ZoomController* zoomController() const { return _zoomController; }

protected:
    /**
     * @brief Handle mouse wheel events for zooming
     * @param event Wheel event
     */
    void wheelEvent(QWheelEvent* event) override;
    
    /**
     * @brief Handle mouse press events for pan initiation
     * @param event Mouse event
     */
    void mousePressEvent(QMouseEvent* event) override;
    
    /**
     * @brief Handle mouse move events for panning
     * @param event Mouse event
     */
    void mouseMoveEvent(QMouseEvent* event) override;
    
    /**
     * @brief Handle mouse release events for pan completion
     * @param event Mouse event
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    ZoomController* _zoomController;  ///< Zoom controller
    bool _isPanning;                   ///< Whether currently panning
    QPoint _lastPanPos;                ///< Last mouse position during pan
};

#endif // ZOOMABLEGRAPHICSVIEW_H
