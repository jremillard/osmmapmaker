#include "styleTab.h"
#include "ui_styleTab.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QColorDialog>

#include <render.h>

#include "newtoplevelstyle.h"
#include "subLayerTextPage.h"

StyleTab::StyleTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StyleTab)
{
	ui->setupUi(this);
	ui->lineTabWidget->addTab(new SubLayerTextPage(this),"Labels");

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
	delete ui;
	project_ = NULL;
	delete render_;
}

void StyleTab::setProject(Project *project)
{
	project_ = NULL;
	project_ = project;

	pixelResolution_ = 0;

	double latitude = 42.608089;
	double lon = -71.571152;

	project_->convertDataToMap(lon, latitude, &centerX_, &centerY_);

	double zoomlevel = 15;
	pixelResolution_ = 156543.03 * cos(latitude * 0.01745329251994329576923690768489) / pow(2,zoomlevel);

	delete render_;
	render_ = NULL;
	render_ = new Render(project_);
}

void StyleTab::showEvent(QShowEvent *event)
{
	updateTree();
}

void StyleTab::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	if (renderedImage_.isNull() == false)
	{
		if ( mouseTracking_)
			painter.drawImage(renderImageLeft()+mouseTrackingCurrent_.x()- mouseTrackingStart_.x(), mouseTrackingCurrent_.y() - mouseTrackingStart_.y(), renderedImage_);
		else
			painter.drawImage(renderImageLeft(), 0, renderedImage_);
	}
}

void StyleTab::on_styleNew_clicked()
{
	if (project_->dataSources().size() == 0)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Please add a data source first."));
		msgBox.exec();
		return;
	}

	NewStopLeveStyle dlg(project_, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString key = dlg.styleKey();
		QString dataSource = dlg.dataSource();
		StyleLayerType type = dlg.styleType();

		StyleLayer *l = new StyleLayer(dataSource,key, type);

		switch (type)
		{
			case ST_POINT:
			{
			}
			break;

			case ST_LINE:
			{
				Line line;
				l->setSubLayerLine(0, line);
			}
			break;

			case ST_AREA:
			{
				Area area;
				l->setSubLayerArea(0, area);
			}
			break;
		}

		
		project_->addStyleLayer(project_->styleLayers().size(), l);
		updateTree();
	}

}

void StyleTab::on_zoomIn_clicked()
{
	renderedImage_ = QImage();

	pixelResolution_ /= 1.5;

	renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);

	repaint();
}


void StyleTab::on_zoomOut_clicked()
{
	renderedImage_ = QImage();

	pixelResolution_ *= 1.5;

	renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);

	repaint();
}

void StyleTab::mouseMoveEvent(QMouseEvent *mevent)
{
	mouseTrackingCurrent_ = mevent->pos();
	if ( mouseTracking_)
		repaint();
}

void StyleTab::mousePressEvent(QMouseEvent *mevent)
{
	mouseTrackingStart_ = mevent->pos();
	mouseTracking_ = true;
}

void StyleTab::mouseReleaseEvent(QMouseEvent *mevent)
{
	if (mouseTracking_)
	{
		mouseTracking_ = false;

		QPoint offset = mouseTrackingCurrent_ - mouseTrackingStart_;

		centerX_ -= offset.x() * pixelResolution_;
		centerY_ += offset.y() * pixelResolution_;

		renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
		repaint();
	}

	mouseTracking_ = false;
}

void StyleTab::resizeEvent(QResizeEvent *mevent)
{
	int zoomButtonsSpacing = 3;

	ui->zoomIn->move(width() - ui->zoomIn->width() - zoomButtonsSpacing, zoomButtonsSpacing);
	ui->zoomOut->move(width() - ui->zoomOut->width() - zoomButtonsSpacing, zoomButtonsSpacing*2+ ui->zoomIn->height());

	ui->styleGroup->resize(ui->styleGroup->width(), height() - zoomButtonsSpacing * 2);

	renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);
	repaint();
}


int StyleTab::renderImageLeft()
{
	return ui->styleGroup->width() + ui->styleGroup->pos().x() + 3;
}

void StyleTab::updateTree()
{
	ui->styleTree->clear();

	QStringList names;
	names.push_back(tr("Map"));
	QTreeWidgetItem *map = new QTreeWidgetItem(names);
	map->setData(0, Qt::UserRole, -1);

	ui->styleTree->addTopLevelItem(map);

	size_t layerIndex = 0;
	for (auto s : project_->styleLayers())
	{
		QString name = tr("%0=*").arg(s->key());
		QStringList names;
		names.push_back(name);

		switch(s->layerType())
		{
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

		QTreeWidgetItem *w = new QTreeWidgetItem(names);
		w->setData(0, Qt::UserRole, layerIndex);

		size_t subLayerIndex = 0;
		for (auto sl : s->subLayerNames())
		{
			QStringList names;
			names.push_back(sl);
			QTreeWidgetItem *subL = new QTreeWidgetItem(names);
			subL->setData(0, Qt::UserRole, subLayerIndex);
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
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	if (currentItem == NULL || (currentItem->parent() == NULL && currentItem->data(0, Qt::UserRole).toInt() < 0))
	{
		ui->styleDetail->setCurrentWidget(ui->pageMap);
		ui->mapBackgroundColor->setText(project_->backgroundColor().name());
	}
	else if (currentItem->parent() == NULL)
	{
		ui->styleDetail->setCurrentWidget(ui->pageLayer);
	}
	else
	{
		std::vector< StyleLayer*> layers = project_->styleLayers();
		size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
		size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

		StyleLayer* layer = layers[index];

		switch (layer->layerType())
		{
			case ST_POINT:
			{
				ui->styleDetail->setCurrentWidget(ui->pagePoint);
				break;
			}

			case ST_LINE:
			{
				ui->styleDetail->setCurrentWidget(ui->pageLine);

				Line line = layer->subLayerLine(subLayerIndex);

				ui->lineVisible->setChecked(line.visible_);
				ui->lineColor->setText(line.color_.name());
				ui->lineWidth->setValue(line.width_);
				ui->lineCasingWidth->setValue(line.casingWidth_);
				ui->lineCasingColor->setText(line.casingColor_.name());
				ui->lineSmooth->setValue(line.smooth_);
				ui->lineOpacity->setValue(line.opacity_);

				break;
			}

			case ST_AREA:
			{
				ui->styleDetail->setCurrentWidget(ui->pageArea);

				Area area = layer->subLayerArea(subLayerIndex);

				ui->areaVisible->setChecked(area.visible_);
				ui->areaColor->setText(area.color_.name());
				ui->areaOpacity->setValue(area.opacity_);

				ui->areaBorderThickness->setValue(area.casingWidth_);
				ui->areaBorderColor->setText(area.casingColor_.name());
				ui->areaFillImage->setText(area.fillImage_);
				ui->areaFillImageOpacity->setValue(area.fillImageOpacity_);

				break;
			}

			default:
				assert(false);
		}
	}
}

////////// map tab
void StyleTab::on_mapUpdateMap_clicked()
{
	freshRender();
}

void StyleTab::on_mapBackgroundColor_editingFinished()
{
	project_->setBackgroundColor(QColor(ui->mapBackgroundColor->text()));
}

void StyleTab::on_mapBackgroundColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->mapBackgroundColor->text()), this);

	if (newColor.isValid())
	{
		ui->mapBackgroundColor->setText(newColor.name());
		on_mapBackgroundColor_editingFinished();
	}
}


////////// layer

void StyleTab::on_layerShowAll_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];
	layer->showAll();

	freshRender();

}

void StyleTab::on_layerHideAll_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];
	layer->hideAll();

	freshRender();

}

void StyleTab::on_layerMoveUp_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->data(0, Qt::UserRole).toUInt();

	if (index > 0)
	{
		StyleLayer* layer = layers[index];

		project_->removeStyleLayer(layer);;

		--index;
		project_->addStyleLayer(index, layer);

		updateTree();
		freshRender();
	}
}

void StyleTab::on_layerMoveDown_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->data(0, Qt::UserRole).toUInt();

	if (index + 1 < layers.size())
	{
		StyleLayer* layer = layers[index];

		project_->removeStyleLayer(layer);;

		++index;
		project_->addStyleLayer(index, layer);

		updateTree();
		freshRender();
	}
}

void StyleTab::on_layerUpdateMap_clicked()
{
	freshRender();
}

void StyleTab::on_layerDelete_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];
	project_->removeStyleLayer(layer);;
	delete layer;

	updateTree();
	freshRender();
	if ( layers.size() == 1)
		ui->styleDetail->setCurrentIndex(0);
}

////////// area tab
void StyleTab::on_areaUpdateMap_clicked()
{
	freshRender();
}

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

void StyleTab::on_areaFillImageOpacity_editedFinished()
{
	saveArea();
}


void StyleTab::on_areaColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->areaColor->text()), this);

	if (newColor.isValid())
	{
		ui->areaColor->setText(newColor.name());
		saveArea();
	}
}

void StyleTab::on_areaBorderColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->areaBorderColor->text()), this);

	if (newColor.isValid())
	{
		ui->areaBorderColor->setText(newColor.name());
		saveArea();
	}
}

void StyleTab::on_areaFillImageSelect_clicked()
{
}

void StyleTab::saveArea()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
	size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];

	//ui->styleDetail->setCurrentWidget(ui->pageLine);

	Area area = layer->subLayerArea(subLayerIndex);

	area.visible_ = ui->areaVisible->isChecked();
	area.color_ = ui->areaColor->text();
	area.opacity_ = ui->areaOpacity->value();

	area.casingWidth_ = ui->areaBorderThickness->value();
	area.casingColor_ = ui->areaBorderColor->text();
	area.fillImage_ = ui->areaFillImage->text();
	area.fillImageOpacity_ = ui->areaFillImageOpacity->value();

	layer->setSubLayerArea(subLayerIndex, area);
}


///////// point tab
void StyleTab::on_pointUpdateMap_clicked()
{
	freshRender();
}

//////// line tab
void StyleTab::on_lineVisible_clicked()
{
	lineSave();
}

void StyleTab::on_lineColor_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineWidth_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineDashArray_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineCasingWidth_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineCasingColor_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineSmooth_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineOpacity_editingFinished()
{
	lineSave();
}

void StyleTab::on_lineCasingColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->lineCasingColor->text()), this);

	if (newColor.isValid())
	{
		ui->lineCasingColor->setText(newColor.name());
		lineSave();
	}
}

void StyleTab::on_lineColorPick_clicked()
{
	QColor newColor = QColorDialog::getColor(QColor(ui->lineColor->text()), this);

	if (newColor.isValid())
	{
		ui->lineColor->setText(newColor.name());
		lineSave();
	}
}

void StyleTab::lineSave()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
	size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];

	//ui->styleDetail->setCurrentWidget(ui->pageLine);

	Line line = layer->subLayerLine(subLayerIndex);

	line.visible_ = ui->lineVisible->isChecked();
	line.color_ = ui->lineColor->text();
	line.width_ = ui->lineWidth->value();
	line.casingWidth_ = ui->lineCasingWidth->value();
	line.casingColor_ = ui->lineCasingColor->text();
	line.smooth_ = ui->lineSmooth->value();
	line.opacity_ = ui->lineOpacity->value();

	layer->setSubLayerLine(subLayerIndex, line);
}

void StyleTab::on_lineUpdateMap_clicked()
{
	freshRender();
}

void StyleTab::freshRender()
{
	delete render_;
	render_ = NULL;
	render_ = new Render(project_);

	renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);

	repaint();
}




