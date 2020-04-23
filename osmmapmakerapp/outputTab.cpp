#include "outputTab.h"
#include "ui_outputTab.h"

OutputTab::OutputTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputTab)
{
	ui->setupUi(this);
}

OutputTab::~OutputTab()
{
	delete ui;
}




