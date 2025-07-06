#include "colorpickerdialog.h"
#include "ui_colorpickerdialog.h"

#include <project.h>
#include <stylelayer.h>
#include <QColorDialog>
#include <QPixmap>
#include <QTableWidgetItem>
#include <algorithm>

struct ColorInfo {
    QColor color;
    QStringList features;
};

QPoint ColorPickerDialog::lastOffset(0, 0);
QSize ColorPickerDialog::lastSize(0, 0);
int ColorPickerDialog::lastSortColumn = 1;
bool ColorPickerDialog::showHint = true;

ColorPickerDialog::ColorPickerDialog(Project* project, const QString& item,
    QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ColorPickerDialog)
{
    ui->setupUi(this);
    project_ = project;
    item_ = item;
    if (!item_.isEmpty())
        setWindowTitle(tr("Select Color - %1").arg(item_));

    if (parent) {
        if (!lastSize.isEmpty()) {
            resize(lastSize);
            move(parent->pos() + lastOffset);
        } else {
            QSize s(std::min(1000, parent->width()), std::min(1000, parent->height()));
            resize(s);
            move(parent->pos());
            lastSize = s;
            lastOffset = QPoint(0, 0);
        }
    } else {
        if (!lastSize.isEmpty())
            resize(lastSize);
        else
            resize(QSize(1000, 1000));
    }

    ui->hintBox->setPlainText(tr("Choose colors that are distinct and readable on maps."));
    ui->hintWidget->setVisible(showHint);

    ui->hueSlider->setRange(0, 359);
    ui->satSlider->setRange(0, 255);
    ui->valSlider->setRange(0, 255);

    connect(ui->hueSlider, &QSlider::valueChanged, this, &ColorPickerDialog::onHueSliderChanged);
    connect(ui->satSlider, &QSlider::valueChanged, this, &ColorPickerDialog::onSatSliderChanged);
    connect(ui->valSlider, &QSlider::valueChanged, this, &ColorPickerDialog::onValSliderChanged);

    connect(ui->hueEdit, &QLineEdit::editingFinished, this, &ColorPickerDialog::onHueEdited);
    connect(ui->satEdit, &QLineEdit::editingFinished, this, &ColorPickerDialog::onSatEdited);
    connect(ui->valEdit, &QLineEdit::editingFinished, this, &ColorPickerDialog::onValEdited);

    connect(ui->htmlColor, &QLineEdit::editingFinished, this, &ColorPickerDialog::onHtmlEdited);
    connect(ui->standardPicker, &QPushButton::clicked, this, &ColorPickerDialog::onStandardPicker);
    connect(ui->dismissHint, &QPushButton::clicked, this, &ColorPickerDialog::onDismissHint);
    connect(ui->colorTable, &QTableWidget::cellClicked, this, &ColorPickerDialog::onColorTableCellClicked);
    connect(ui->colorTable->horizontalHeader(), &QHeaderView::sectionClicked, this, &ColorPickerDialog::onHeaderClicked);

    populateColors();
    updatePatch(QColor());
}

ColorPickerDialog::~ColorPickerDialog() { }

QColor ColorPickerDialog::selectedColor() const { return color_; }

void ColorPickerDialog::setCurrentColor(const QColor& color)
{
    color_ = color;
    int h, s, v;
    color.getHsv(&h, &s, &v);
    if (h < 0)
        h = 0;
    ui->hueSlider->blockSignals(true);
    ui->satSlider->blockSignals(true);
    ui->valSlider->blockSignals(true);
    ui->hueSlider->setValue(h);
    ui->satSlider->setValue(s);
    ui->valSlider->setValue(v);
    ui->hueSlider->blockSignals(false);
    ui->satSlider->blockSignals(false);
    ui->valSlider->blockSignals(false);

    ui->hueEdit->setText(QString::number(h, 16));
    ui->satEdit->setText(QString::number(s, 16));
    ui->valEdit->setText(QString::number(v, 16));
    ui->htmlColor->setText(color.name());
    updatePatch(color);

    int row = findRow(color);
    if (row >= 0) {
        ui->colorTable->selectRow(row);
        ui->colorTable->scrollToItem(ui->colorTable->item(row, 0));
    }
}

QColor ColorPickerDialog::getColor(Project* project, const QColor& initial,
    const QString& item, QWidget* parent)
{
    ColorPickerDialog dlg(project, item, parent);
    dlg.setCurrentColor(initial);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.selectedColor();
    return QColor();
}

void ColorPickerDialog::populateColors()
{
    if (!project_)
        return;

    QMap<QString, ColorInfo> colorMap;
    auto addColor = [&colorMap](const QColor& c, const QString& feature) {
        QString key = c.name();
        auto it = colorMap.find(key);
        if (it == colorMap.end()) {
            ColorInfo info;
            info.color = c;
            info.features << feature;
            colorMap.insert(key, info);
        } else {
            if (!it->features.contains(feature))
                it->features << feature;
        }
    };

    addColor(project_->backgroundColor(), tr("Background"));

    for (StyleLayer* layer : project_->styleLayers()) {
        QString layerKey = layer->key();
        std::vector<QString> names = layer->subLayerNames();
        for (size_t i = 0; i < layer->subLayerCount(); ++i) {
            QString prefix = QString("%1/%2").arg(layerKey, names[i]);
            switch (layer->layerType()) {
            case ST_POINT: {
                Point p = layer->subLayerPoint(i);
                addColor(p.color_, prefix + tr(" point"));
                break;
            }
            case ST_LINE: {
                Line l = layer->subLayerLine(i);
                addColor(l.color_, prefix + tr(" line"));
                addColor(l.casingColor_, prefix + tr(" line casing"));
                break;
            }
            case ST_AREA: {
                Area a = layer->subLayerArea(i);
                addColor(a.color_, prefix + tr(" area"));
                addColor(a.casingColor_, prefix + tr(" area border"));
                break;
            }
            }
            Label lb = layer->label(i);
            addColor(lb.color_, prefix + tr(" label"));
            addColor(lb.haloColor_, prefix + tr(" halo"));
        }
    }

    std::vector<ColorInfo> colors;
    for (auto it = colorMap.begin(); it != colorMap.end(); ++it)
        colors.push_back(*it);

    ui->colorTable->setColumnCount(4);
    ui->colorTable->setHorizontalHeaderLabels({ tr("Color"), tr("Hue"), tr("Saturation"), tr("Value") });
    ui->colorTable->setRowCount(static_cast<int>(colors.size()));

    for (size_t i = 0; i < colors.size(); ++i) {
        const ColorInfo& info = colors[i];
        QPixmap pm(20, 20);
        pm.fill(info.color);
        QTableWidgetItem* itemColor = new QTableWidgetItem(QIcon(pm), "");
        itemColor->setData(Qt::UserRole, info.color);
        itemColor->setToolTip(info.features.join(", "));
        int h, s, v;
        info.color.getHsv(&h, &s, &v);
        if (h < 0)
            h = 0;
        ui->colorTable->setItem(i, 0, itemColor);
        ui->colorTable->setItem(i, 1, new QTableWidgetItem(QString::number(h)));
        ui->colorTable->setItem(i, 2, new QTableWidgetItem(QString::number(s)));
        ui->colorTable->setItem(i, 3, new QTableWidgetItem(QString::number(v)));
    }
    ui->colorTable->setSortingEnabled(true);
    ui->colorTable->sortItems(lastSortColumn);
}

int ColorPickerDialog::findRow(const QColor& color) const
{
    for (int r = 0; r < ui->colorTable->rowCount(); ++r) {
        QColor c = ui->colorTable->item(r, 0)->data(Qt::UserRole).value<QColor>();
        if (c == color)
            return r;
    }
    return -1;
}

void ColorPickerDialog::onColorTableCellClicked(int row, int /*column*/)
{
    QColor c = ui->colorTable->item(row, 0)->data(Qt::UserRole).value<QColor>();
    setCurrentColor(c);
}

void ColorPickerDialog::onHueSliderChanged(int v)
{
    int h = v;
    int s = ui->satSlider->value();
    int val = ui->valSlider->value();
    QColor c;
    c.setHsv(h, s, val);
    setCurrentColor(c);
}

void ColorPickerDialog::onSatSliderChanged(int v)
{
    onHueSliderChanged(ui->hueSlider->value());
}

void ColorPickerDialog::onValSliderChanged(int v)
{
    onHueSliderChanged(ui->hueSlider->value());
}

void ColorPickerDialog::onHueEdited()
{
    bool ok;
    int v = ui->hueEdit->text().toInt(&ok, 16);
    if (ok)
        ui->hueSlider->setValue(v);
}

void ColorPickerDialog::onSatEdited()
{
    bool ok;
    int v = ui->satEdit->text().toInt(&ok, 16);
    if (ok)
        ui->satSlider->setValue(v);
}

void ColorPickerDialog::onValEdited()
{
    bool ok;
    int v = ui->valEdit->text().toInt(&ok, 16);
    if (ok)
        ui->valSlider->setValue(v);
}

void ColorPickerDialog::onHtmlEdited()
{
    QColor c(ui->htmlColor->text());
    if (c.isValid())
        setCurrentColor(c);
}

void ColorPickerDialog::onStandardPicker()
{
    QColor c = QColorDialog::getColor(color_, this);
    if (c.isValid())
        setCurrentColor(c);
}

void ColorPickerDialog::onDismissHint()
{
    ui->hintWidget->setVisible(false);
    showHint = false;
}

void ColorPickerDialog::onHeaderClicked(int index)
{
    lastSortColumn = index;
    // preserve selection
    QColor c = color_;
    ui->colorTable->sortItems(index);
    int row = findRow(c);
    if (row >= 0) {
        ui->colorTable->selectRow(row);
        ui->colorTable->scrollToItem(ui->colorTable->item(row, 0));
    }
}

void ColorPickerDialog::updatePatch(const QColor& color)
{
    QPixmap pm(60, 60);
    pm.fill(color);
    ui->currentColorPatch->setPixmap(pm);
    ui->htmlColor->setText(color.name());
}

void ColorPickerDialog::moveEvent(QMoveEvent* event)
{
    QDialog::moveEvent(event);
    if (parentWidget())
        lastOffset = pos() - parentWidget()->pos();
    else
        lastOffset = pos();
}

void ColorPickerDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    lastSize = size();
}
