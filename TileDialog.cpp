#include "TileDialog.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>

// DimOverlay implementation
DimOverlay::DimOverlay(TileDialog* dialog)
    : QGraphicsRectItem()
    , _dialog(dialog)
{
    setBrush(QColor(0, 0, 0, 180));  // Semi-transparent black
    setPen(Qt::NoPen);
    setZValue(-1);  // Behind dialog but above everything else
    setAcceptedMouseButtons(Qt::LeftButton);
}

void DimOverlay::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Close dialog when clicking on dim overlay (outside dialog)
    if (_dialog && event->button() == Qt::LeftButton) {
        _dialog->hide();
        _dialog->rejected();
        event->accept();
    } else {
        QGraphicsRectItem::mousePressEvent(event);
    }
}

// TileDialog implementation

TileDialog::TileDialog(float widthMultiplier, float heightMultiplier, int gridY, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, Anchor::Center, 0, gridY, parent)
    , _dimOverlay(nullptr)
    , _proxyWidget(nullptr)
    , _scrollArea(nullptr)
    , _contentWidget(nullptr)
{
    // Dialogs start hidden
    QGraphicsItem::setVisible(false);
}

TileDialog::~TileDialog()
{
    // _dimOverlay is owned by the scene (added via scene()->addItem())
    // Proxy widget is owned by this dialog (parent relationship)
    // Qt's parent-child ownership handles cleanup automatically
}

void TileDialog::show(float baseTileSize, float sceneWidth)
{
    if (!scene()) {
        return;
    }
    
    // Create dim overlay if it doesn't exist
    if (!_dimOverlay) {
        _dimOverlay = new DimOverlay(this);
        scene()->addItem(_dimOverlay);
    }
    
    // Size overlay to cover entire scene
    QRectF sceneRect = scene()->sceneRect();
    _dimOverlay->setRect(sceneRect);
    _dimOverlay->setVisible(true);
    
    // Update dialog geometry (this also updates children)
    updateGeometry(baseTileSize, sceneWidth);
    
    // Update proxy widget geometry if we have content
    if (_proxyWidget) {
        updateProxyWidgetGeometry();
    }
    
    // Bring dialog to front
    setZValue(1000);
    
    // Show dialog
    QGraphicsItem::setVisible(true);
    setFocus();
}

void TileDialog::hide()
{
    QGraphicsItem::setVisible(false);
    
    if (_dimOverlay) {
        _dimOverlay->setVisible(false);
    }
}

bool TileDialog::isVisible() const
{
    return QGraphicsItem::isVisible();
}

void TileDialog::updateGeometry(float baseTileSize, float sceneWidth, int fontSize)
{
    // Update dialog's own geometry
    Tile::updateGeometry(baseTileSize, sceneWidth, fontSize);
    
    // Update dim overlay size if it exists (regardless of visibility)
    if (_dimOverlay && scene()) {
        _dimOverlay->setRect(scene()->sceneRect());
    }
    
    // Update all child tiles
    for (QGraphicsItem* child : childItems()) {
        if (Tile* tile = dynamic_cast<Tile*>(child)) {
            tile->updateGeometry(baseTileSize, _width, fontSize);  // Use dialog width as scene width for children
        }
    }
    
    // Update proxy widget geometry if we have content
    if (_proxyWidget) {
        updateProxyWidgetGeometry();
    }
}

void TileDialog::setContentWidget(QWidget* content)
{
    if (!content) {
        return;
    }
    
    _contentWidget = content;
    
    // Create scroll area if it doesn't exist
    if (!_scrollArea) {
        setupScrollArea();
    }
    
    // Set content in scroll area
    _scrollArea->setWidget(_contentWidget);
    
    // Update proxy widget geometry if already visible
    if (isVisible()) {
        updateProxyWidgetGeometry();
    }
}

QWidget* TileDialog::contentWidget() const
{
    return _contentWidget;
}

void TileDialog::setupScrollArea()
{
    // Create scroll area
    _scrollArea = new QScrollArea();
    _scrollArea->setWidgetResizable(true);  // Let it resize the widget to fit
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setFrameShape(QFrame::NoFrame);
    
    // Style the scroll area to match theme
    _scrollArea->setStyleSheet(
        "QScrollArea {"
        "   background-color: transparent;"
        "   border: none;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: rgba(40, 40, 40, 200);"
        "   width: 12px;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: rgba(100, 100, 100, 200);"
        "   border-radius: 6px;"
        "   min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: rgba(120, 120, 150, 220);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "   background-color: rgba(40, 40, 40, 200);"
        "   height: 12px;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "   background-color: rgba(100, 100, 100, 200);"
        "   border-radius: 6px;"
        "   min-width: 20px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "   background-color: rgba(120, 120, 150, 220);"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "   width: 0px;"
        "}"
    );
    
    // Create proxy widget to host the scroll area in the graphics scene
    _proxyWidget = new QGraphicsProxyWidget(this);
    _proxyWidget->setWidget(_scrollArea);
    _proxyWidget->setZValue(1);  // Above dialog background
    
    // Prevent proxy from auto-resizing based on widget size hints
    _proxyWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    
    // Force the dialog to clip its children to its bounding rect
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
}

void TileDialog::updateProxyWidgetGeometry()
{
    if (!_proxyWidget) {
        return;
    }
    
    QRectF rect = boundingRect();
    
    // Leave some padding around the edges for the dialog border
    const qreal padding = 10;
    qreal proxyX = padding;
    qreal proxyY = padding;
    qreal proxyWidth = rect.width() - 2 * padding;
    qreal proxyHeight = rect.height() - 2 * padding;
    
    // Set scroll area to fixed size FIRST, before setting proxy geometry
    if (_scrollArea) {
        QSize targetSize(static_cast<int>(proxyWidth), static_cast<int>(proxyHeight));
        _scrollArea->setFixedSize(targetSize);
    }
    
    // Now set proxy widget geometry - it should respect the scroll area's fixed size
    _proxyWidget->setGeometry(QRectF(proxyX, proxyY, proxyWidth, proxyHeight));
    
    // Update font sizes and styling when geometry changes
    updateContentWidgetStyle();
}

void TileDialog::updateContentWidgetStyle()
{
    if (!_contentWidget || _currentFontSize == 0) {
        return;
    }
    
    // Use centralized font size from MainWindow
    int baseFontSize = _currentFontSize;
    int labelFontSize = static_cast<int>(baseFontSize * 1.3f);  // Slightly larger for labels (matching TileLabel)
    
    // Recursively update all child widgets with proper font sizes
    QList<QWidget*> children = _contentWidget->findChildren<QWidget*>();
    children.prepend(_contentWidget);  // Include content widget itself
    
    for (QWidget* widget : children) {
        QFont font = widget->font();
        font.setPixelSize(baseFontSize);
        font.setBold(true);  // All fonts bold to match tile style
        widget->setFont(font);
        
        // Labels get slightly larger, bold font (matching TileLabel style)
        if (QLabel* label = qobject_cast<QLabel*>(widget)) {
            font.setPixelSize(labelFontSize);
            font.setBold(true);
            label->setFont(font);
        }
    }
}

void TileDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
        emit rejected();
        event->accept();
    } else {
        Tile::keyPressEvent(event);
    }
}

void TileDialog::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    
    // Draw the dialog background (lighter than normal tiles)
    painter->setRenderHint(QPainter::Antialiasing);
    
    QRectF rect = boundingRect();
    float radius = cornerRadius(_currentBaseTileSize);
    
    // Dialog background - lighter, more opaque
    painter->setBrush(QColor(60, 60, 60, 240));
    painter->setPen(QPen(QColor(100, 100, 100), 2));
    painter->drawRoundedRect(rect, radius, radius);
}
