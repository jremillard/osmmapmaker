#include "dataTab.h"
#include "ui_dataTab.h"

DataTab::DataTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DataTab)
{
	ui->setupUi(this);
}

DataTab::~DataTab()
{
	delete ui;
}







