#include "styleTab.h"
#include "ui_styleTab.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include "colorpickerdialog.h"

#include <renderqt.h>
#include "maputils.h"

#include "newtoplevelstyle.h"

StyleTab::StyleTab(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StyleTab)
{
    supressTreeSelection_ = false;

    lineLabelPage_ = NULL;
    areaLabelPage_ = NULL;
    pointLabelPage_ = NULL;
    lineSelectPage_ = NULL;
    areaSelectPage_ = NULL;
    pointSelectPage_ = NULL;

    ui->setupUi(this);

    lineLabelPage_ = new SubLayerTextPage(this);
    areaLabelPage_ = new SubLayerTextPage(this);
    pointLabelPage_ = new SubLayerTextPage(this);

    bool r = connect(lineLabelPage_, &SubLayerTextPage::editingFinished, this, &StyleTab::on_editingFinishedLineLabel);
    assert(r);
    connect(areaLabelPage_, &SubLayerTextPage::editingFinished, this, &StyleTab::on_editingFinishedAreaLabel);
    assert(r);
    connect(pointLabelPage_, &SubLayerTextPage::editingFinished, this, &StyleTab::on_editingFinishedPointLabel);
    assert(r);

    ui->lineTabWidget->addTab(lineLabelPage_, "Labels");
    ui->areaTabWidget->addTab(areaLabelPage_, "Labels");
    ui->pointTabWidget->addTab(pointLabelPage_, "Labels");

    lineSelectPage_ = new SubLayerSelectPage(this);
    areaSelectPage_ = new SubLayerSelectPage(this);
    pointSelectPage_ = new SubLayerSelectPage(this);

    r = connect(lineSelectPage_, &SubLayerSelectPage::editingFinished, this, &StyleTab::on_editingFinishedLineLabel);
    assert(r);
    r = connect(areaSelectPage_, &SubLayerSelectPage::editingFinished, this, &StyleTab::on_editingFinishedAreaLabel);
    assert(r);
    r = connect(pointSelectPage_, &SubLayerSelectPage::editingFinished, this, &StyleTab::on_editingFinishedPointLabel);
    assert(r);

    ui->lineTabWidget->addTab(lineSelectPage_, "Select");
    ui->areaTabWidget->addTab(areaSelectPage_, "Select");
    ui->pointTabWidget->addTab(pointSelectPage_, "Select");

    project_ = NULL;

    centerX_ = 0;
    centerY_ = 0;
    pixelResolution_ = 0;
    mouseTracking_ = false;

    ui->styleDetail->setCurrentIndex(0);
    render_ = NULL;
}

StyleTab::~StyleTab()
{
    delete lineLabelPage_;
    delete areaLabelPage_;
    delete pointLabelPage_;

    delete lineSelectPage_;
    delete areaSelectPage_;
    delete pointSelectPage_;

    delete ui;
    project_ = NULL;
    delete render_;
}

void StyleTab::setProject(Project* project)
{
    project_ = NULL;
    project_ = project;

    pixelResolution_ = 0;

    double latitude = 42.608089;
    double lon = -71.571152;

    project_->convertDataToMap(lon, latitude, &centerX_, &centerY_);

    ui->zoom->setValue(13);
    updatePixelResolutionFromZoom();

    lineLabelPage_->setProject(project_);
    areaLabelPage_->setProject(project_);
    pointLabelPage_->setProject(project_);

    delete render_;
    render_ = NULL;
    render_ = new RenderQT(project_);
}

void StyleTab::showEvent(QShowEvent* event)
{
    updateTree();
}

void StyleTab::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (renderedImage_.isNull() == false) {
        QRect background(renderImageLeft(), 0, renderedImage_.width(), renderedImage_.height());
        painter.setBrush(QBrush(QColor(project_->backgroundColor())));
        painter.drawRect(background);
        if (mouseTracking_)
            painter.drawImage(renderImageLeft() + mouseTrackingCurrent_.x() - mouseTrackingStart_.x(), mouseTrackingCurrent_.y() - mouseTrackingStart_.y(), renderedImage_);
        else
            painter.drawImage(renderImageLeft(), 0, renderedImage_);
    }
}

void StyleTab::on_treeNew_clicked()
{
    if (project_->dataSources().size() == 0) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Please add a data source first."));
        msgBox.exec();
        return;
    }

    bool toplevelStyle = false;

    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    if (project_->styleLayers().size() == 0 || currentItem == NULL || currentItem->data(0, Qt::UserRole).toInt() < 0) {
        // Always do top level style if project is empty, nothing is selected, or the map node is selected.
        // otherwise, don't know what style to stick the sub style on.
        toplevelStyle = true;
    } else {
        // Otherwise we need to ask what kind of new style
    }

    if (toplevelStyle) {
        // Top level style, don't need
        NewStopLeveStyle dlg(project_, this);
        if (dlg.exec() == QDialog::Accepted) {
            QString key = dlg.styleKey();
            QString dataSource = dlg.dataSource();
            StyleLayerType type = dlg.styleType();

            StyleLayer* l = new StyleLayer(dataSource, key, type);

            switch (type) {
            case ST_POINT: {
                Point point;
                l->setSubLayerPoint(0, point);
            } break;

            case ST_LINE: {
                Line line;
                l->setSubLayerLine(0, line);
            } break;

            case ST_AREA: {
                Area area;
                l->setSubLayerArea(0, area);
            } break;
            }

            // stick at the top so people can see it.
            project_->addStyleLayer(0, l);
            updateTree();
        }
    } else if (currentItem->parent() == NULL) {
        int layerIndex = currentItem->data(0, Qt::UserRole).toInt();

        std::vector<StyleLayer*> layers = project_->styleLayers();

        StyleLayer* layer = layers[layerIndex];

        size_t insertIndex = layer->subLayerNames().size();

        switch (layer->dataType()) {
        case OET_POINT: {
            Point point;
            point.color_ = QColor(Qt::red);
            point.width_ = 2;
            point.opacity_ = 1.0;

            layer->setSubLayerPoint(insertIndex, point);

        } break;

        case OET_LINE: {
            Line line;
            line.color_ = QColor(Qt::red);
            line.width_ = 2;
            line.opacity_ = 0.5;

            layer->setSubLayerLine(insertIndex, line);

        } break;
        case OET_AREA: {
            Area area;
            area.color_ = QColor(Qt::red);
            area.opacity_ = 0.5;

            layer->setSubLayerArea(insertIndex, area);
        }

        break;
        }

        Label label;
        label.visible_ = true;
        label.text_ = QString("[name] + ' - ' + [%0]").arg(layer->key());

        layer->setLabel(insertIndex, label);

        updateTree();

        for (int i = 0; i < ui->styleTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = ui->styleTree->topLevelItem(i);
            if (item->data(0, Qt::UserRole).toInt() == layerIndex) {
                ui->styleTree->setCurrentItem(item);
                item->setExpanded(true);
                break;
            }
        }
    }
}

void StyleTab::on_treeCopy_clicked()
{
}

void StyleTab::on_treeDelete_clicked()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    if (currentItem != NULL) {
        std::vector<StyleLayer*> layers = project_->styleLayers();

        if (currentItem->parent() == NULL) {
            // top level layer delete.
            int index = currentItem->data(0, Qt::UserRole).toInt();

            if (index >= 0 && index < layers.size()) {

                StyleLayer* layer = layers[index];
                project_->removeStyleLayer(layer);
                ;
                delete layer;

                updateTree();

                for (int i = 0; i < ui->styleTree->topLevelItemCount(); ++i) {
                    QTreeWidgetItem* item = ui->styleTree->topLevelItem(i);
                    if (item->data(0, Qt::UserRole).toInt() == index) {
                        ui->styleTree->setCurrentItem(item);
                        break;
                    }
                }
            } else {
                // they tried to delete the map node
            }
        } else {
            size_t selectedLayerIndex = currentItem->parent()->data(0, Qt::UserRole).toUInt();

            StyleLayer* layer = layers[selectedLayerIndex];

            size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

            if (layer->subLayerCount() > 1) {

                layer->removeSubLayer(subLayerIndex);

                updateTree();

                for (int layerIndex = 0; layerIndex < ui->styleTree->topLevelItemCount(); ++layerIndex) {
                    QTreeWidgetItem* parentItem = ui->styleTree->topLevelItem(layerIndex);

                    if (parentItem->data(0, Qt::UserRole).toUInt() == selectedLayerIndex) {
                        parentItem->setExpanded(true);

                        for (int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex) {
                            QTreeWidgetItem* subLayerItem = parentItem->child(childIndex);

                            if (subLayerItem->data(0, Qt::UserRole).toUInt() == subLayerIndex) {
                                ui->styleTree->setCurrentItem(subLayerItem);
                                break;
                            }
                        }

                        break;
                    }
                }
            } else {
                // Lame hack: don't delete the last layer; users can select the top level node if this is what they want.
            }
        }
    }
}

void StyleTab::on_treeUp_clicked()
{
    moveTreeItem(-1);
}

void StyleTab::on_treeDown_clicked()
{
    moveTreeItem(1);
}

void StyleTab::moveTreeItem(int direction)
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();

    if (currentItem != NULL && currentItem->parent() != NULL) {
        size_t selectedLayerIndex = currentItem->parent()->data(0, Qt::UserRole).toUInt();

        StyleLayer* layer = layers[selectedLayerIndex];

        size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

        if (subLayerIndex + direction >= 0 && subLayerIndex + direction < layer->subLayerNames().size()) {
            layer->subLayerMove(subLayerIndex, direction);

            updateTree();

            for (int layerIndex = 0; layerIndex < ui->styleTree->topLevelItemCount(); ++layerIndex) {
                QTreeWidgetItem* parentItem = ui->styleTree->topLevelItem(layerIndex);

                if (parentItem->data(0, Qt::UserRole).toUInt() == selectedLayerIndex) {
                    parentItem->setExpanded(true);

                    for (int childIndex = 0; childIndex < parentItem->childCount(); ++childIndex) {
                        QTreeWidgetItem* subLayerItem = parentItem->child(childIndex);

                        if (subLayerItem->data(0, Qt::UserRole).toUInt() == subLayerIndex + direction) {
                            ui->styleTree->setCurrentItem(subLayerItem);
                            break;
                        }
                    }

                    break;
                }
            }
        }
    } else if (currentItem != NULL && currentItem->parent() == NULL) {
        size_t index = currentItem->data(0, Qt::UserRole).toUInt();

        if (index + direction < layers.size() && index + direction >= 0) {
            StyleLayer* layer = layers[index];

            project_->removeStyleLayer(layer);
            ;

            index += direction;
            project_->addStyleLayer(index, layer);

            updateTree();

            for (int i = 0; i < ui->styleTree->topLevelItemCount(); ++i) {
                QTreeWidgetItem* item = ui->styleTree->topLevelItem(i);
                if (item->data(0, Qt::UserRole).toInt() == index) {
                    ui->styleTree->setCurrentItem(item);
                    break;
                }
            }
        }
    }
}

void StyleTab::on_zoom_editingFinished()
{
    /*
    renderedImage_ = QImage();

    updatePixelResolutionFromZoom();

    render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
    renderedImage_ = render_->RenderImage();

    repaint();
    */
}

void StyleTab::on_zoomIn_clicked()
{
    renderedImage_ = QImage();

    ui->zoom->setValue(ui->zoom->value() + 1);

    updatePixelResolutionFromZoom();

    render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
    renderedImage_ = render_->RenderImage();

    repaint();
}

void StyleTab::on_zoomOut_clicked()
{
    renderedImage_ = QImage();

    ui->zoom->setValue(ui->zoom->value() - 1);

    updatePixelResolutionFromZoom();

    render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
    renderedImage_ = render_->RenderImage();

    repaint();
}

void StyleTab::updatePixelResolutionFromZoom()
{
    double latitude, lon;
    project_->convertMapToData(centerX_, centerY_, &lon, &latitude);

    pixelResolution_ = metersPerPixel(latitude, ui->zoom->value());
}

void StyleTab::mouseMoveEvent(QMouseEvent* mevent)
{
    mouseTrackingCurrent_ = mevent->pos();
    if (mouseTracking_)
        repaint();
}

void StyleTab::mousePressEvent(QMouseEvent* mevent)
{
    mouseTrackingStart_ = mevent->pos();
    mouseTracking_ = true;
}

void StyleTab::mouseReleaseEvent(QMouseEvent* mevent)
{
    if (mouseTracking_) {
        mouseTracking_ = false;

        QPoint offset = mouseTrackingCurrent_ - mouseTrackingStart_;

        centerX_ -= offset.x() * pixelResolution_;
        centerY_ += offset.y() * pixelResolution_;

        render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
        renderedImage_ = render_->RenderImage();
        repaint();
    }

    mouseTracking_ = false;
}

void StyleTab::resizeEvent(QResizeEvent* mevent)
{
    int zoomButtonsSpacing = 3;

    ui->zoomIn->move(width() - ui->zoomIn->width() - zoomButtonsSpacing,
        zoomButtonsSpacing);
    ui->zoomOut->move(
        width() - ui->zoomOut->width() - zoomButtonsSpacing,
        zoomButtonsSpacing * 2 + ui->zoomIn->height());
    ui->zoom->move(
        width() - ui->zoomIn->width() - zoomButtonsSpacing * 2 - ui->zoom->width(),
        zoomButtonsSpacing + ui->zoomIn->height() / 2);

    ui->styleGroup->resize(ui->styleGroup->width(), height() - zoomButtonsSpacing * 2);

    render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
    renderedImage_ = render_->RenderImage();

    repaint();
}

int StyleTab::renderImageLeft()
{
    return ui->styleGroup->width() + ui->styleGroup->pos().x() + 3;
}

void StyleTab::updateTree()
{
    supressTreeSelection_ = true;
    ui->styleTree->clear();
    supressTreeSelection_ = false;

    QStringList names;
    names.push_back(tr("Map"));
    QTreeWidgetItem* map = new QTreeWidgetItem(names);
    map->setData(0, Qt::UserRole, -1);

    ui->styleTree->addTopLevelItem(map);

    size_t layerIndex = 0;
    for (auto s : project_->styleLayers()) {
        QString name = tr("%0").arg(s->key());
        QStringList names;
        names.push_back(name);

        switch (s->layerType()) {
        case ST_POINT:
            names.push_back(tr("Point"));
            break;
        case ST_LINE:
            names.push_back(tr("Line"));
            break;
        case ST_AREA:
            names.push_back(tr("Area"));
            break;
        default:
            assert(false);
        }

        QTreeWidgetItem* w = new QTreeWidgetItem(names);
        w->setData(0, Qt::UserRole, static_cast<qulonglong>(layerIndex));

        size_t subLayerIndex = 0;
        for (auto sl : s->subLayerNames()) {
            QStringList names;
            names.push_back(sl);
            QTreeWidgetItem* subL = new QTreeWidgetItem(names);
            subL->setData(0, Qt::UserRole, static_cast<qulonglong>(subLayerIndex));
            w->addChild(subL);
            ++subLayerIndex;
        }

        ui->styleTree->addTopLevelItem(w);

        ++layerIndex;
    }

    int iconWidth = 22;
    int typeWidth = 20;
    ui->styleTree->setColumnWidth(0, ui->styleTree->width() - iconWidth - typeWidth);
    ui->styleTree->setColumnWidth(1, typeWidth);

    on_styleTree_itemSelectionChanged();
}

void StyleTab::on_styleTree_itemSelectionChanged()
{
    if (supressTreeSelection_)
        return;

    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    if (currentItem == NULL || (currentItem->parent() == NULL && currentItem->data(0, Qt::UserRole).toInt() < 0)) {
        ui->styleDetail->setCurrentWidget(ui->pageMap);
        ui->mapBackgroundColor->setText(project_->backgroundColor().name());
        ui->mapBackgroundOpacity->setValue(project_->backgroundOpacity());
    } else if (currentItem->parent() == NULL) {
        ui->styleDetail->setCurrentWidget(ui->pageLayer);
    } else {
        std::vector<StyleLayer*> layers = project_->styleLayers();
        size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
        size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

        StyleLayer* layer = layers[index];

        switch (layer->layerType()) {
        case ST_POINT: {
            ui->styleDetail->setCurrentWidget(ui->pagePoint);

            Point point = layer->subLayerPoint(subLayerIndex);

            ui->pointVisible->setChecked(point.visible_);
            ui->pointMinZoom->setValue(point.minZoom_);
            ui->pointColor->setText(point.color_.name());
            ui->pointOpacity->setValue(point.opacity_);
            ui->pointImage->setText(point.image_);
            ui->pointWidth->setValue(point.width_);

            pointLabelPage_->Load(layer->label(subLayerIndex));
            pointSelectPage_->Load(project_->renderDatabase(), layer->dataSource(), layer->subLayerSelectors(subLayerIndex));

            break;
        }

        case ST_LINE: {
            ui->styleDetail->setCurrentWidget(ui->pageLine);

            Line line = layer->subLayerLine(subLayerIndex);

            ui->lineVisible->setChecked(line.visible_);
            ui->lineMinZoom->setValue(line.minZoom_);
            ui->lineColor->setText(line.color_.name());
            ui->lineWidth->setValue(line.width_);
            ui->lineCasingWidth->setValue(line.casingWidth_);
            ui->lineCasingColor->setText(line.casingColor_.name());
            ui->lineSmooth->setValue(line.smooth_);
            ui->lineOpacity->setValue(line.opacity_);

            QString dashArrayStr = "";
            for (auto pair : line.dashArray_) {
                if (dashArrayStr.length() > 0)
                    dashArrayStr += ", ";
                dashArrayStr += QString::number(pair.first) + "," + QString::number(pair.second);
            }
            ui->lineDashArray->setText(dashArrayStr);

            lineLabelPage_->Load(layer->label(subLayerIndex));
            lineSelectPage_->Load(project_->renderDatabase(), layer->dataSource(), layer->subLayerSelectors(subLayerIndex));

            break;
        }

        case ST_AREA: {
            ui->styleDetail->setCurrentWidget(ui->pageArea);

            Area area = layer->subLayerArea(subLayerIndex);

            ui->areaVisible->setChecked(area.visible_);
            ui->areaMinZoom->setValue(area.minZoom_);
            ui->areaColor->setText(area.color_.name());
            ui->areaOpacity->setValue(area.opacity_);

            ui->areaBorderThickness->setValue(area.casingWidth_);
            ui->areaBorderColor->setText(area.casingColor_.name());
            ui->areaFillImage->setText(area.fillImage_);
            ui->areaFillImageOpacity->setValue(area.fillImageOpacity_);

            areaLabelPage_->Load(layer->label(subLayerIndex));
            areaSelectPage_->Load(project_->renderDatabase(), layer->dataSource(), layer->subLayerSelectors(subLayerIndex));

            break;
        }

        default:
            assert(false);
        }

        Label label = layer->label(subLayerIndex);
    }
}

////////// map tab
void StyleTab::on_updateMap_clicked()
{
    freshRender();
}

void StyleTab::on_mapBackgroundColor_editingFinished()
{
    project_->setBackgroundColor(QColor(ui->mapBackgroundColor->text()));
}

void StyleTab::on_mapBackgroundOpacity_valueChanged(double v)
{
    project_->setBackgroundOpacity(v);
}

void StyleTab::on_mapBackgroundColorPick_clicked()
{
    QString old = ui->mapBackgroundColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("map background"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->mapBackgroundColor->setText(newColor.name());
        on_mapBackgroundColor_editingFinished();
        freshRender();
    }
}

////////// layer

void StyleTab::on_layerShowAll_clicked()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();
    size_t index = currentItem->data(0, Qt::UserRole).toUInt();

    StyleLayer* layer = layers[index];
    layer->showAll();
    freshRender();
}

void StyleTab::on_layerHideAll_clicked()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();
    size_t index = currentItem->data(0, Qt::UserRole).toUInt();

    StyleLayer* layer = layers[index];
    layer->hideAll();
    freshRender();
}

////////// area tab

void StyleTab::on_areaVisible_clicked()
{
    saveArea();
}

void StyleTab::on_areaColor_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaOpacity_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaBorderThickness_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaBorderColor_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaFillImage_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaFillImageOpacity_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaMinZoom_editingFinished()
{
    saveArea();
}

void StyleTab::on_areaColorPick_clicked()
{
    QString old = ui->areaColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("area fill"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->areaColor->setText(newColor.name());
        saveArea();
        freshRender();
    }
}

void StyleTab::on_areaBorderColorPick_clicked()
{
    QString old = ui->areaBorderColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("area border"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->areaBorderColor->setText(newColor.name());
        saveArea();
        freshRender();
    }
}

void StyleTab::on_areaFillImageSelect_clicked()
{
}

void StyleTab::on_editingFinishedAreaLabel()
{
    saveArea();
}

void StyleTab::saveArea()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();
    size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
    size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

    StyleLayer* layer = layers[index];

    // ui->styleDetail->setCurrentWidget(ui->pageLine);

    Area area = layer->subLayerArea(subLayerIndex);

    area.visible_ = ui->areaVisible->isChecked();
    area.minZoom_ = ui->areaMinZoom->value();
    area.color_ = ui->areaColor->text();
    area.opacity_ = ui->areaOpacity->value();

    area.casingWidth_ = ui->areaBorderThickness->value();
    area.casingColor_ = ui->areaBorderColor->text();
    area.fillImage_ = ui->areaFillImage->text();
    area.fillImageOpacity_ = ui->areaFillImageOpacity->value();

    Label label = layer->label(subLayerIndex);
    areaLabelPage_->SaveTo(&label);
    layer->setLabel(subLayerIndex, label);

    StyleSelector select = layer->subLayerSelectors(subLayerIndex);
    areaSelectPage_->SaveTo(&select);
    layer->setSubLayerSelectors(subLayerIndex, select);

    layer->setSubLayerArea(subLayerIndex, area);
}

///////// point tab

void StyleTab::on_editingFinishedPointLabel()
{
    savePoint();
}

void StyleTab::on_pointVisible_clicked()
{
    savePoint();
}

void StyleTab::on_pointColor_editingFinished()
{
    savePoint();
}

void StyleTab::on_pointColorPick_clicked()
{
    QString old = ui->pointColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("point"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->pointColor->setText(newColor.name());
        savePoint();
        freshRender();
    }
}

void StyleTab::on_pointWidth_editingFinished()
{
    savePoint();
}

void StyleTab::on_pointOpacity_editingFinished()
{
    savePoint();
}

void StyleTab::savePoint()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();
    size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
    size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

    StyleLayer* layer = layers[index];

    // ui->styleDetail->setCurrentWidget(ui->pageLine);

    Point point = layer->subLayerPoint(subLayerIndex);

    point.visible_ = ui->pointVisible->isChecked();
    point.color_ = ui->pointColor->text();
    point.minZoom_ = ui->pointMinZoom->value();
    point.width_ = ui->pointWidth->value();
    point.opacity_ = ui->pointOpacity->value();
    point.image_ = ui->pointImage->text();

    layer->setSubLayerPoint(subLayerIndex, point);

    Label lb = layer->label(subLayerIndex);
    pointLabelPage_->SaveTo(&lb);
    layer->setLabel(subLayerIndex, lb);

    StyleSelector select = layer->subLayerSelectors(subLayerIndex);
    pointSelectPage_->SaveTo(&select);
    layer->setSubLayerSelectors(subLayerIndex, select);
}

//////// line tab
void StyleTab::on_lineVisible_clicked()
{
    saveLine();
}

void StyleTab::on_lineColor_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineWidth_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineDashArray_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineCasingWidth_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineCasingColor_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineSmooth_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineOpacity_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineMinZoom_editingFinished()
{
    saveLine();
}

void StyleTab::on_lineCasingColorPick_clicked()
{
    QString old = ui->lineCasingColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("line casing"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->lineCasingColor->setText(newColor.name());
        saveLine();
        freshRender();
    }
}

void StyleTab::on_lineColorPick_clicked()
{
    QString old = ui->lineColor->text();
    QColor newColor = ColorPickerDialog::getColor(project_, QColor(old), tr("line"), this);

    if (newColor.isValid() && newColor.name() != old) {
        ui->lineColor->setText(newColor.name());
        saveLine();
        freshRender();
    }
}

void StyleTab::on_editingFinishedLineLabel()
{
    saveLine();
}

void StyleTab::saveLine()
{
    QTreeWidgetItem* currentItem = ui->styleTree->currentItem();

    std::vector<StyleLayer*> layers = project_->styleLayers();
    size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
    size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

    StyleLayer* layer = layers[index];

    // ui->styleDetail->setCurrentWidget(ui->pageLine);

    Line line = layer->subLayerLine(subLayerIndex);

    line.visible_ = ui->lineVisible->isChecked();
    line.color_ = ui->lineColor->text();
    line.minZoom_ = ui->lineMinZoom->value();
    line.width_ = ui->lineWidth->value();
    line.casingWidth_ = ui->lineCasingWidth->value();
    line.casingColor_ = ui->lineCasingColor->text();
    line.smooth_ = ui->lineSmooth->value();
    line.opacity_ = ui->lineOpacity->value();

    QStringList dashArray = ui->lineDashArray->text().split(",");
    for (QString& dash : dashArray) {
        dash = dash.trimmed();
    }

    line.dashArray_.clear();
    for (size_t i = 0; i + 1 < dashArray.size(); i += 2) {
        line.dashArray_.push_back(std::pair<double, double>(dashArray[i].toDouble(), dashArray[i + 1].toDouble()));
    }

    layer->setSubLayerLine(subLayerIndex, line);

    Label lb = layer->label(subLayerIndex);
    lineLabelPage_->SaveTo(&lb);
    layer->setLabel(subLayerIndex, lb);

    StyleSelector select = layer->subLayerSelectors(subLayerIndex);
    lineSelectPage_->SaveTo(&select);
    layer->setSubLayerSelectors(subLayerIndex, select);
}

void StyleTab::freshRender()
{
    delete render_;
    render_ = NULL;
    render_ = new RenderQT(project_);

    render_->SetupZoomAtCenter(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
    renderedImage_ = render_->RenderImage();

    repaint();
}
