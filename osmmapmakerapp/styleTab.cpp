#include "styleTab.h"
#include "ui_styleTab.h"

#include <QPainter>
#include <QMouseEvent>

#include <render.h>

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
	delete render_;
	render_ = NULL;
	render_ = new Render(project);

	renderedImage_ = render_->RenderImage(width(), height(), centerX_, centerY_, pixelResolution_);

	render_->GetBoundingBox(&centerX_, &centerY_, &pixelResolution_);
}

void StyleTab::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	if (renderedImage_.isNull() == false)
	{
		if ( mouseTracking_)
			painter.drawImage(mouseTrackingCurrent_.x()- mouseTrackingStart_.x(), mouseTrackingCurrent_.y() - mouseTrackingStart_.y(), renderedImage_);
		else
			painter.drawImage(0, 0, renderedImage_);
	}
}

void StyleTab::on_zoomIn_clicked()
{
	renderedImage_ = QImage();

	pixelResolution_ /= 1.5;

	renderedImage_ = render_->RenderImage(width(), height(), centerX_, centerY_, pixelResolution_);

	repaint();
}

void StyleTab::on_zoomOut_clicked()
{
	renderedImage_ = QImage();

	pixelResolution_ *= 1.5;

	renderedImage_ = render_->RenderImage(width(), height(), centerX_, centerY_, pixelResolution_);

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

		renderedImage_ = render_->RenderImage(width(), height(), centerX_, centerY_, pixelResolution_);
		repaint();
	}

	mouseTracking_ = false;
}

void StyleTab::resizeEvent(QResizeEvent *mevent)
{
	int zoomButtonsSpacing = 3;

	ui->zoomIn->move(width() - ui->zoomIn->width() - zoomButtonsSpacing, zoomButtonsSpacing);
	ui->zoomOut->move(width() - ui->zoomOut->width() - zoomButtonsSpacing, zoomButtonsSpacing*2+ ui->zoomIn->height());

	renderedImage_ = render_->RenderImage(width(), height(), centerX_, centerY_, pixelResolution_);
	repaint();
}










