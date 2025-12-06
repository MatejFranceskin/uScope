#include "TileCombo.h"

TileCombo::TileCombo(float widthMultiplier, float heightMultiplier, Anchor anchor, QGraphicsItem* parent)
    : Tile(widthMultiplier, heightMultiplier, anchor, parent)
{
    // Create QComboBox
    _comboBox = new QComboBox();
    _comboBox->setMinimumHeight(30);
    
    // Create proxy widget to embed QComboBox in QGraphicsScene
    _proxyWidget = new QGraphicsProxyWidget(this);
    _proxyWidget->setWidget(_comboBox);
    
    // Forward signals
    connect(_comboBox, &QComboBox::currentTextChanged, this, &TileCombo::currentTextChanged);
    connect(_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TileCombo::currentIndexChanged);
}

void TileCombo::addItem(const QString& text, const QVariant& userData)
{
    _comboBox->addItem(text, userData);
}

void TileCombo::setCurrentIndex(int index)
{
    _comboBox->setCurrentIndex(index);
}

int TileCombo::currentIndex() const
{
    return _comboBox->currentIndex();
}

QString TileCombo::currentText() const
{
    return _comboBox->currentText();
}

void TileCombo::updateGeometry(float baseTileSize, int positionIndex)
{
    Tile::updateGeometry(baseTileSize, positionIndex);
    
    // Resize proxy widget to match tile size
    _proxyWidget->resize(boundingRect().size());
    
    // Apply styling to combo box based on tile size
    int fontSize = static_cast<int>(baseTileSize * 0.12f);
    QString styleSheet = QString(
        "QComboBox {"
        "   background-color: rgba(40, 40, 40, 200);"
        "   color: white;"
        "   border: none;"
        "   border-radius: %1px;"
        "   padding: 5px;"
        "   font-size: %2px;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "}"
        "QComboBox::down-arrow {"
        "   image: url(:/images/arrow-down.svg);"
        "   width: %3px;"
        "   height: %3px;"
        "}"
    ).arg(static_cast<int>(cornerRadius(baseTileSize)))
     .arg(fontSize)
     .arg(static_cast<int>(baseTileSize * 0.15f));
    
    _comboBox->setStyleSheet(styleSheet);
}
