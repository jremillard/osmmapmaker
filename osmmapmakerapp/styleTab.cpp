#include "styleTab.h"
#include "ui_styleTab.h"

#include <QPainter>
#include <QMouseEvent>

#include <render.h>

#include "newtoplevelstyle.h"

StyleTab::StyleTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StyleTab)
{
	ui->setupUi(this);
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
	project_ = project;

	updateTree();
	
	freshRender();
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
	NewStopLeveStyle dlg(project_, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		QString key = dlg.styleKey();
		StyleLayerType type = dlg.styleType();

		StyleLayer *l = new StyleLayer(key, type);

		switch (type)
		{
			case ST_LABEL:
			{
			}
			break;

			case ST_POINT:
			{
			}
			break;

			case ST_LINE:
			{
				Line line;
				line.casingWidth_ = 0;
				line.casingColor_ = QColor(Qt::white);
				line.color_ = QColor(Qt::black);
				line.width_ = 1.5;

				l->setSubLayerLine(0, line);
				std::vector<StyleSelector> selectors;
				StyleSelector sel;
				sel.SetAnds(std::vector<QString>() = { "track","access" }, std::vector<QString>() = { "yes","no" });
				
				selectors.push_back(sel);
				l->setSubLayerSelectors(0, selectors);
			}
			break;

			case ST_AREA:
			{
			}
			break;
		}

		project_->addStyleLayer(l);
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
	ui->styleTree->reset();
	ui->styleTree->clear();


	size_t layerIndex = 0;
	for (auto s : project_->styleLayers())
	{
		QString name = tr("%0=*").arg(s->key());
		QStringList names;
		names.push_back(name);

		switch(s->layerType())
		{
			case ST_LABEL:
				names.push_back(tr("Label"));
				break;
			case ST_POINT:
				names.push_back(tr("Point"));
				break;
			case ST_LINE:
				names.push_back(tr("Line"));
				break;
			case ST_AREA:
				names.push_back(tr("Area"));
				break;
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
}

void StyleTab::on_styleTree_itemSelectionChanged()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	if (currentItem == NULL)
	{
		ui->styleDetail->setCurrentWidget(ui->emptyStylePage);
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

			case ST_LABEL:
			{
				ui->styleDetail->setCurrentWidget(ui->emptyStylePage);
				break;
			}

			case ST_POINT:
			{
				ui->styleDetail->setCurrentWidget(ui->pagePoint);
				break;
			}

			case ST_LINE:
			{
				ui->styleDetail->setCurrentWidget(ui->pageLine);

				Line line = layer->subLayerLine(subLayerIndex);

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
				break;
			}
		}
	}
}

void StyleTab::on_lineUpdate_clicked()
{
	QTreeWidgetItem *currentItem = ui->styleTree->currentItem();

	std::vector< StyleLayer*> layers = project_->styleLayers();
	size_t index = currentItem->parent()->data(0, Qt::UserRole).toUInt();
	size_t subLayerIndex = currentItem->data(0, Qt::UserRole).toUInt();

	StyleLayer* layer = layers[index];

	ui->styleDetail->setCurrentWidget(ui->pageLine);

	Line line = layer->subLayerLine(subLayerIndex);

	line.color_ = ui->lineColor->text();
	line.width_ = ui->lineWidth->value();
	line.casingWidth_ = ui->lineCasingWidth->value();
	line.casingColor_ = ui->lineCasingColor->text();
	line.smooth_= ui->lineSmooth->value();
	line.opacity_ = ui->lineOpacity->value();

	layer->setSubLayerLine(subLayerIndex, line);

	freshRender();
}

void StyleTab::freshRender()
{
	delete render_;
	render_ = NULL;
	render_ = new Render(project_);

	if (pixelResolution_ <= 0 || pixelResolution_ != pixelResolution_)
		render_->GetBoundingBox(&centerX_, &centerY_, &pixelResolution_);

	renderedImage_ = render_->RenderImage(width() - renderImageLeft(), height(), centerX_, centerY_, pixelResolution_);

	if (pixelResolution_ <= 0 || pixelResolution_ != pixelResolution_)
		render_->GetBoundingBox(&centerX_, &centerY_, &pixelResolution_);

	repaint();
}




