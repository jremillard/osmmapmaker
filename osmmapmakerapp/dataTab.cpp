#include "dataTab.h"
#include "ui_dataTab.h"

#include <QMessageBox>


DataTab::DataTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DataTab)
{
	project_ = NULL;
	ui->setupUi(this);
}

DataTab::~DataTab()
{
	delete ui;
}

void DataTab::setProject(Project *project)
{
	project_ = project;

	ui->outputs->clear();
	UpdateDataSourceList();
}

void DataTab::UpdateDataSourceList()
{
	if (project_ != NULL)
	{
		ui->deleteData->setEnabled(true);

		for (DataSource *output : project_->dataSources())
		{
			QDateTime importTime = output->importTime();
			QString name;
			if (importTime.isNull())
			{
				name = QString("%1 - Not Imported").arg(output->userName());
			}
			else
			{
				QDateTime now = QDateTime::currentDateTime();

				long long diffS = importTime.secsTo(now);

				QString when;
				if (diffS < 60)
					when = "Now";
				else if (diffS < 60 * 60)
					when = QString("%1 Minutes Ago").arg(diffS / 60);
				else if (diffS < 60 * 60 * 24)
					when = QString("%1 Hours Ago").arg(diffS / (60 * 60));
				else if (diffS < 60 * 60 * 24 * 15)
					when = QString("%1 Days Ago").arg(diffS / (60 * 60 * 24));
				else
					when = importTime.toString();

				when = importTime.toString();

				name = QString("%1 - Imported %2").arg(output->userName());
			}

			ui->outputs->addItem(name);
		}
	}
	else
	{
		ui->deleteData->setEnabled(false);
	}
}

void DataTab::on_deleteData_clicked()
{
	QMessageBox msgBox(this);
	msgBox.setText(QString("delete"));
	msgBox.exec();

}

void DataTab::on_add_clicked()
{
	QString importName;
	try
	{
		path renderDbPath = project_->renderDatabasePath();
		QString nativePath = QString::fromStdWString(renderDbPath.native());

		SQLite::Database db(nativePath.toUtf8().constBegin(), SQLite::OPEN_READWRITE);

		for (DataSource *output : project_->dataSources())
		{
			importName = output->userName();

			SQLite::Transaction transaction(db);

			SQLite::Statement removeDataStatement(db, "DELETE FROM entity WHERE source = ?");
			removeDataStatement.bind(1, output->dataName().toStdString());
			removeDataStatement.exec();

			output->importData(db);
			transaction.commit();
		}
	}
	catch (std::exception &e)
	{
		QMessageBox msgBox;
		msgBox.setText(QString("Failure importing %1 data.\n%2").arg(importName, e.what()));
		msgBox.exec();
	}
}









