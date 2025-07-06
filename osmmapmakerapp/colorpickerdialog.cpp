#include "colorpickerdialog.h"
#include "ui_colorpickerdialog.h"

#include <project.h>
#include <stylelayer.h>
#include <QListWidgetItem>
#include <QPixmap>
#include <algorithm>

namespace {
static QPoint lastOffset(0, 0);
static QSize lastSize(0, 0);
struct ColorInfo {
    QColor color;
    QStringList features;
};
}

ColorPickerDialog::ColorPickerDialog(Project* project, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ColorPickerDialog)
{
    ui->setupUi(this);
    project_ = project;

    if (parent) {
        if (!lastSize.isEmpty()) {
            resize(lastSize);
            move(parent->pos() + lastOffset);
        } else {
            QSize s(std::max(1000, parent->width()), std::max(1000, parent->height()));
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

    populateColors();
    updatePatch(QColor());
}

ColorPickerDialog::~ColorPickerDialog() { }

QColor ColorPickerDialog::selectedColor() const { return color_; }

void ColorPickerDialog::setCurrentColor(const QColor& color)
{
    color_ = color;
    ui->colorWidget->setCurrentColor(color);
    updatePatch(color);
    for (int i = 0; i < ui->usedColors->count(); ++i) {
        QListWidgetItem* item = ui->usedColors->item(i);
        QColor c = item->data(Qt::UserRole).value<QColor>();
        if (c == color) {
            ui->usedColors->setCurrentItem(item);
            ui->usedColors->scrollToItem(item);
            break;
        }
    }
}

QColor ColorPickerDialog::getColor(Project* project, const QColor& initial, QWidget* parent)
{
    ColorPickerDialog dlg(project, parent);
    dlg.setCurrentColor(initial);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.selectedColor();
    return QColor();
}

void ColorPickerDialog::populateColors()
{
    if (!project_)
        return;

    // map color hex -> info
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
    std::sort(colors.begin(), colors.end(), [](const ColorInfo& a, const ColorInfo& b) {
        int ha = a.color.hslHue();
        int hb = b.color.hslHue();
        if (ha < 0)
            ha = 361;
        if (hb < 0)
            hb = 361;
        return ha < hb;
    });

    for (const ColorInfo& info : colors) {
        QString label = info.features.join(", ");
        QListWidgetItem* item = new QListWidgetItem(label);
        QPixmap pm(20, 20);
        pm.fill(info.color);
        item->setIcon(QIcon(pm));
        item->setData(Qt::UserRole, info.color);
        item->setToolTip(label);
        ui->usedColors->addItem(item);
    }
}

void ColorPickerDialog::on_usedColors_itemClicked(QListWidgetItem* item)
{
    QColor c = item->data(Qt::UserRole).value<QColor>();
    setCurrentColor(c);
}

void ColorPickerDialog::on_usedColors_itemDoubleClicked(QListWidgetItem* item)
{
    on_usedColors_itemClicked(item);
    accept();
}

void ColorPickerDialog::on_colorWidget_currentColorChanged(const QColor& color)
{
    color_ = color;
    updatePatch(color);
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
