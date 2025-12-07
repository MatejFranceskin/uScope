#ifndef ZOOMSTATE_H
#define ZOOMSTATE_H

#include <QObject>
#include <QPointF>
#include <QTransform>

/**
 * Represents the current zoom and pan state of the video view
 * Zoom factor range: 0.1x (min, fit to viewport) to 10.0x (max)
 */
class ZoomState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor NOTIFY zoomFactorChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QPointF panOffset READ panOffset WRITE setPanOffset NOTIFY panOffsetChanged)

public:
    enum Mode {
        Custom,      // User-defined zoom factor
        FitWidth,    // Scale to fit viewport width
        FitHeight,   // Scale to fit viewport height
        OneToOne     // 1:1 pixel mapping (100%)
    };
    Q_ENUM(Mode)

    explicit ZoomState(QObject* parent = nullptr);

    qreal zoomFactor() const { return _zoomFactor; }
    Mode mode() const { return _mode; }
    QPointF panOffset() const { return _panOffset; }

    /**
     * Calculate the view transform based on current zoom and pan state
     * @param viewportSize Size of the viewport
     * @param contentSize Size of the content (video frame)
     * @return QTransform to apply to the view
     */
    QTransform calculateViewTransform(const QSizeF& viewportSize, const QSizeF& contentSize) const;

    /**
     * Get display string for current zoom state (e.g., "2.3x", "Fit W", "100%")
     */
    QString displayString() const;

public slots:
    void setZoomFactor(qreal factor);
    void setMode(Mode mode);
    void setPanOffset(const QPointF& offset);
    
    /**
     * Set zoom factor without changing mode (internal use)
     */
    void setZoomFactorInternal(qreal factor);

    /**
     * Reset to default state (fit to viewport)
     */
    void reset();

signals:
    void zoomFactorChanged(qreal factor);
    void modeChanged(Mode mode);
    void panOffsetChanged(const QPointF& offset);
    void stateChanged();

private:
    qreal clampZoomFactor(qreal factor) const;

    qreal _zoomFactor;
    Mode _mode;
    QPointF _panOffset;

    static constexpr qreal MIN_ZOOM = 0.1;
    static constexpr qreal MAX_ZOOM = 10.0;
};

#endif // ZOOMSTATE_H
