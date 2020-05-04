#include "styleTab.h"
#include "ui_styleTab.h"

#include <QPainter>

#include <render.h>

StyleTab::StyleTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StyleTab)
{
	ui->setupUi(this);
	project_ = NULL;

}

StyleTab::~StyleTab()
{
	delete ui;
	project_ = NULL;
}

void StyleTab::setProject(Project *project)
{
	project_ = project;
}

void StyleTab::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	if (renderedImage_.isNull() == false)
	{
		painter.drawImage(0, 100, renderedImage_);
	}
}

void StyleTab::on_updateMap_clicked()
{
	renderedImage_ = QImage();
	Render r(project_, 0, 0, 19.11, width(), height());

	renderedImage_ = r.RenderImage();

	repaint();
}








