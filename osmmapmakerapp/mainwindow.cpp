#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dataTab.h"
#include "styleTab.h"
#include "outputTab.h"

#include "project.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->tabWidget->addTab(new DataTab(this), tr("Data"));
	ui->tabWidget->addTab(new StyleTab(this), tr("Style"));
	ui->tabWidget->addTab(new OutputTab(this), tr("Output"));

       project_ = NULL;

       try
       {
               openProject("C:\\Remillard\\Documents\\osmmapmaker\\projects\\groton-trail.osmmap.xml");
       }
       catch (std::exception &)
       {
               auto locs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
               QString loc;
               if (locs.size() > 0)
                       loc = locs[0];

               QString file = QFileDialog::getOpenFileName(this, tr("Open Project"), loc, tr("Map Project Files (*.xml)"));

               if (file.isEmpty())
               {
                       qApp->exit();
                       return;
               }

               openProject(file.toStdString());
       }

       setCursor(Qt::ArrowCursor);
}


void MainWindow::openProject(path projectPath)
{
	delete project_;
	project_ = NULL;
	project_ = new Project(projectPath);
    setWindowTitle(QString("OSM Map Maker - %1").arg(QString::fromStdWString(projectPath.filename().wstring())));

	DataTab *dataTab = dynamic_cast<DataTab*>(ui->tabWidget->widget(0));
	dataTab->setProject(project_);

	StyleTab *styleTab = dynamic_cast<StyleTab*>(ui->tabWidget->widget(1));
	styleTab->setProject(project_);

	OutputTab *outputTab = dynamic_cast<OutputTab *>(ui->tabWidget->widget(2));
	outputTab->setProject(project_);

	if (project_->dataSources().size() == 0)
	{
		ui->tabWidget->setCurrentIndex(0);
	}
	else
	{
		ui->tabWidget->setCurrentIndex(1);
	}
}

MainWindow::~MainWindow()
{
    delete ui;
	delete project_;
}

void MainWindow::on_action_Project_New_triggered()
{
	QMessageBox msgBox(this);
	msgBox.setText(QString("New"));
	msgBox.exec();
}

void MainWindow::on_action_Project_Open_triggered()
{
	auto locs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
	QString loc;
	if (locs.size() > 0)
		loc = locs[0];

	QString file = QFileDialog::getOpenFileName(this, tr("Open Project"), loc, tr("Map Project Files (*.xml)"));

	if (file.isEmpty() == false)
	{
		QMessageBox msgBox(this);

		msgBox.setText(QString("Open %1").arg(file));
		msgBox.exec();
	}

}

void MainWindow::on_action_Project_Copy_triggered()
{
	QMessageBox msgBox(this);

	msgBox.setText(QString("Copy"));
	msgBox.exec();

}

void MainWindow::on_action_Project_Save_triggered()
{
	try
	{
		setCursor(Qt::WaitCursor);

		if (project_ != NULL)
			project_->save();

		setCursor(Qt::ArrowCursor);
	}
	catch (std::exception &e)
	{
		setCursor(Qt::ArrowCursor);

		QMessageBox msgBox;
		msgBox.setText(QString("Failure saving %1 project.\n%2").arg(QString::fromStdString(project_->projectPath().string()), e.what()));
		msgBox.exec();
	}

}

void MainWindow::on_actionExit_triggered()
{
	this->close();
}
