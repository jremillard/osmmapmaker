#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dataTab.h"
#include "styleTab.h"
#include "outputTab.h"

#include "project.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->tabWidget->addTab(new DataTab(this), tr("Data"));
	ui->tabWidget->addTab(new StyleTab(this), tr("Style"));
	ui->tabWidget->addTab(new OutputTab(this), tr("Output"));

	project_ = new project("c:\\remillard\\documents\\osmmapmaker\\project.xml");
}

MainWindow::~MainWindow()
{
    delete ui;
	delete project_;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
