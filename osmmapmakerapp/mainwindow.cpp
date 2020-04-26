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

	project_ = NULL;

	openProject("C:\\Users\\Jason\\Documents\\Map Maker\\project.xml");
}


void MainWindow::openProject(path projectPath)
{
	delete project_;
	project_ = NULL;
	project_ = new Project(projectPath);
	setWindowTitle(QString("OSM Map Maker - %1").arg(projectPath.filename().wstring()));

	DataTab *dataTab = dynamic_cast<DataTab*>(ui->tabWidget->widget(0));
	dataTab->setProject(project_);
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