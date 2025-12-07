#ifndef ZOOMCONTROLLER_H
#define ZOOMCONTROLLER_H

#include <QObject>
#include <QPointF>
#include <QSizeF>
#include <QTransform>

class ZoomState;
class QGraphicsView;
class VideoGraphicsScene;

/**
 * @brief Controller coordinating zoom and pan operations
 * 
 * ZoomController acts as the coordination layer between user input
 * (mouse wheel, gestures, buttons) and the ZoomState model. It manages
 * the zoom state and applies the resulting transform to the video background
 * layer only, keeping tiles at their original positions.
 * 
 * Responsibilities:
 * - Handle zoom factor changes with validation
 * - Coordinate zoom mode changes (Custom, FitWidth, FitHeight, OneToOne)
 * - Manage pan offset updates
 * - Apply calculated transforms to the QGraphicsView
 * - Emit signals for UI updates
 * 
 * Usage:
 * @code
 * ZoomController* controller = new ZoomController(graphicsView, this);
 * connect(wheelEvent, &..., controller, &ZoomController::zoomIn);
 * connect(fitWidthButton, &..., controller, &ZoomController::setFitWidth);
 * @endcode
 */
class ZoomController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new ZoomController
     * @param view The QGraphicsView to apply zoom transforms to
     * @param parent Parent QObject for memory management
     */
    explicit ZoomController(QGraphicsView* view, QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ZoomController() override;

    /**
     * @brief Get the current zoom state model
     * @return Pointer to the ZoomState instance
     */
    ZoomState* zoomState() const { return _zoomState; }

    /**
     * @brief Set the content size for zoom calculations
     * 
     * This should be called whenever the video frame size or content
     * dimensions change. Used for FitWidth/FitHeight calculations.
     * 
     * @param size The size of the content being displayed
     */
    void setContentSize(const QSizeF& size);

    /**
     * @brief Get the current content size
     * @return The content size in pixels
     */
    QSizeF contentSize() const { return _contentSize; }

public slots:
    /**
     * @brief Set zoom factor directly
     * @param factor Zoom factor (0.1x to 10.0x, will be clamped)
     */
    void setZoomFactor(qreal factor);

    /**
     * @brief Increment zoom by 0.1x
     * 
     * Typically called by mouse wheel scroll up events.
     */
    void zoomIn();

    /**
     * @brief Decrement zoom by 0.1x
     * 
     * Typically called by mouse wheel scroll down events.
     */
    void zoomOut();

    /**
     * @brief Set zoom mode to FitWidth
     * 
     * Scales content to fit viewport width exactly.
     */
    void setFitWidth();

    /**
     * @brief Set zoom mode to FitHeight
     * 
     * Scales content to fit viewport height exactly.
     */
    void setFitHeight();

    /**
     * @brief Set zoom mode to OneToOne (100%)
     * 
     * Displays content at actual pixel size (1:1 mapping).
     */
    void setOneToOne();

    /**
     * @brief Set pan offset
     * @param offset Pan offset in scene coordinates
     */
    void setPanOffset(const QPointF& offset);

    /**
     * @brief Set zoom factor and pan offset together
     * @param factor Zoom factor (0.1x to 10.0x, will be clamped)
     * @param offset Pan offset in scene coordinates
     */
    void setZoomAndPan(qreal factor, const QPointF& offset);

    /**
     * @brief Pan by a delta amount
     * @param delta Amount to pan in scene coordinates
     */
    void panBy(const QPointF& delta);

    /**
     * @brief Reset zoom and pan to default (FitWidth, no pan)
     */
    void reset();
    
    /**
     * @brief Update zoom transform after viewport size change
     * 
     * Should be called when the view is resized to recalculate
     * the transform based on the new viewport dimensions.
     */
    void updateViewportSize();

signals:
    /**
     * @brief Emitted when zoom factor or mode changes
     * @param factor New zoom factor
     * @param mode New zoom mode
     */
    void zoomChanged(qreal factor, int mode);

    /**
     * @brief Emitted when pan offset changes
     * @param offset New pan offset
     */
    void panChanged(const QPointF& offset);

    /**
     * @brief Emitted when zoom state changes
     * 
     * Generic signal for any state change (zoom or pan).
     */
    void stateChanged();

private slots:
    /**
     * @brief Handle ZoomState changes and update view
     */
    void onZoomStateChanged();

private:
    /**
     * @brief Apply current zoom transform to the video background layer
     */
    void applyTransform();

    ZoomState* _zoomState;            ///< Zoom state model
    QGraphicsView* _view;              ///< Graphics view (for viewport size)
    VideoGraphicsScene* _scene;        ///< Graphics scene (for video transform)
    QSizeF _contentSize;               ///< Content size for fit calculations
    bool _updatingTransform;           ///< Flag to prevent recursive transform updates
};

#endif // ZOOMCONTROLLER_H
