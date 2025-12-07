#include "TileSlider.h"
#include <QVBoxLayout>
#include <QWidget>

TileSlider::TileSlider(Qt::Orientation orientation, float widthMultiplier, 
                       float heightMultiplier, Anchor anchor, int gridX, int gridY, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, anchor, gridX, gridY, parent)
    , _orientation(orientation)
{
    // Create container widget
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Create label
    _label = new QLabel();
    _label->setAlignment(Qt::AlignCenter);
    _label->setStyleSheet("color: white;");
    layout->addWidget(_label);
    
    // Create slider
    _slider = new QSlider(orientation);
    _slider->setMinimumHeight(20);
    layout->addWidget(_slider);
    
    container->setLayout(layout);
    
    // Create proxy widget to embed in QGraphicsScene
    _proxyWidget = new QGraphicsProxyWidget(this);
    _proxyWidget->setWidget(container);
    
    // Forward signal
    connect(_slider, &QSlider::valueChanged, this, &TileSlider::valueChanged);
}

void TileSlider::setRange(int min, int max)
{
    _slider->setRange(min, max);
}

void TileSlider::setValue(int value)
{
    _slider->setValue(value);
}

int TileSlider::value() const
{
    return _slider->value();
}

void TileSlider::setLabel(const QString& label)
{
    _label->setText(label);
}

void TileSlider::updateGeometry(float baseTileSize, float sceneWidth)
{
    Tile::updateGeometry(baseTileSize, sceneWidth);
    
    // Resize proxy widget to match tile size
    _proxyWidget->resize(boundingRect().size());
    
    // Apply styling based on tile size
    int fontSize = static_cast<int>(baseTileSize * 0.12f);
    QString styleSheet = QString(
        "QSlider::groove:horizontal {"
        "   background: rgba(60, 60, 60, 200);"
        "   height: 8px;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: rgba(0, 120, 215, 255);"
        "   width: %1px;"
        "   height: %1px;"
        "   margin: -8px 0;"
        "   border-radius: %2px;"
        "}"
        "QSlider::groove:vertical {"
        "   background: rgba(60, 60, 60, 200);"
        "   width: 8px;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:vertical {"
        "   background: rgba(0, 120, 215, 255);"
        "   width: %1px;"
        "   height: %1px;"
        "   margin: 0 -8px;"
        "   border-radius: %2px;"
        "}"
    ).arg(static_cast<int>(baseTileSize * 0.2f))
     .arg(static_cast<int>(baseTileSize * 0.1f));
    
    _slider->setStyleSheet(styleSheet);
    
    _label->setStyleSheet(QString("color: white; font-size: %1px;").arg(fontSize));
}
