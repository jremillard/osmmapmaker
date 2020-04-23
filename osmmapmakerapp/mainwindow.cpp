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


	project *p = new project();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
