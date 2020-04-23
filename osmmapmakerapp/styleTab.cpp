#include "styleTab.h"
#include "ui_styleTab.h"

StyleTab::StyleTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StyleTab)
{
	ui->setupUi(this);
}

StyleTab::~StyleTab()
{
	delete ui;
}







