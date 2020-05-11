#include "dataTab.h"
#include "ui_dataTab.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <osmdatafile.h>

DataTab::DataTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DataTab)
{
	project_ = NULL;
	ui->setupUi(this);
	currentIndex_ = -1;
}

DataTab::~DataTab()
{
	delete ui;
}

void DataTab::setProject(Project *project)
{
	project_ = project;

	UpdateDataSourceList();
}

void DataTab::UpdateDataSourceList()
{
	ui->dataSources->clear();

	if (project_ != NULL)
	{
		for (DataSource *output : project_->dataSources())
		{
			ui->dataSources->addItem(output->userName());
		}
	}

	on_dataSources_currentIndexChanged(ui->dataSources->currentIndex());
}

void DataTab::on_dataSources_currentIndexChanged(int index)
{
	ui->stackedWidget->setCurrentWidget(ui->emptyPage);

	if (currentIndex_ != index)
	{
		// save it.
		saveCurrent();
	}

	currentIndex_ = index;

	ui->stackedWidget->setCurrentWidget(ui->emptyPage);
	ui->lastImportDate->clear();
	ui->dataSourceNameID->clear();
	ui->dataSouceIDEdit->setEnabled(false);
	ui->dataSourceUserRename->setEnabled(false);

	if ( index >= 0)
	{
		QString name = ui->dataSources->currentText();

		for (DataSource *output : project_->dataSources())
		{
			ui->dataSouceIDEdit->setEnabled(true);
			ui->dataSourceUserRename->setEnabled(true);

			if (name == output->userName())
			{
				if (dynamic_cast<OsmDataFile*>(output) != NULL)
				{
					ui->stackedWidget->setCurrentWidget(ui->pageOSMFile);

					OsmDataFile *outputC = dynamic_cast<OsmDataFile*>(output);

					ui->OSMFileName->setText(QDir::toNativeSeparators(outputC->localFile()));
				}

				QDateTime importTime = output->importTime();
				QString when;
				if (importTime.isNull())
				{
					ui->lastImportDate->setText( tr("Not Imported Yet"));
				}
				else
				{
					QDateTime now = QDateTime::currentDateTime();

					long long diffS = importTime.secsTo(now);

					if (diffS < 60*2)
						when = "Now";
					else if (diffS < 60 * 60*2)
						when = QString("%1 Minutes Ago").arg(diffS / 60);
					else if (diffS < 60 * 60 * 24)
						when = QString("%1 Hours Ago").arg(diffS / (60 * 60));
					else if (diffS < 60 * 60 * 24 * 15)
						when = QString("%1 Days Ago").arg(diffS / (60 * 60 * 24));
					else
						when = importTime.toString();

					ui->lastImportDate->setText(tr("%1 - %2").arg(when, importTime.toString()));
				}

				ui->dataSourceNameID->setText(output->dataName());
			}
		}
	}
}

void DataTab::on_OSMFileDelete_clicked()
{
	QString name = ui->dataSources->currentText();

	for (DataSource *output : project_->dataSources())
	{
		if (name == output->userName())
		{
			project_->removeDataSource(output);
			UpdateDataSourceList();
			break;
		}
	}
}

void DataTab::on_OSMFileBrowse_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open OSM File"), "", tr("Image Files (*.osm.pbf *.osm)"));

	if (file.isEmpty() == false)
	{
		ui->OSMFileName->setText(QDir::toNativeSeparators(file));
		saveCurrent();
	}
}


void DataTab::on_OSMFileImport_clicked()
{
	QString importName;
	try
	{
		setCursor(Qt::WaitCursor);

		saveCurrent();

		QString name = ui->dataSources->currentText();

		for (DataSource *output : project_->dataSources())
		{
			if (name == output->userName())
			{
				SQLite::Database *db = project_->renderDatabase();

				importName = output->userName();

				SQLite::Transaction transaction(*db);

				SQLite::Statement removeDataStatement(*db, "DELETE FROM entity WHERE source = ?");
				removeDataStatement.bind(1, output->dataName().toStdString());
				removeDataStatement.exec();

				output->importData(*db);
				transaction.commit();

				project_->createViews();

				QDateTime now = QDateTime::currentDateTime();

				output->setImportTime(now);

				on_dataSources_currentIndexChanged(ui->dataSources->currentIndex());
				break;
			}
		}

		setCursor(Qt::ArrowCursor);
	}
	catch (std::exception &e)
	{
		this->setCursor(Qt::ArrowCursor);

		QMessageBox msgBox;
		msgBox.setText(QString("Failure importing %1 data.\n%2").arg(importName, e.what()));
		msgBox.exec();
	}
}

void DataTab::on_OSMFileName_textChanged(QString text)
{
	if (QFileInfo::exists(text))
	{
		ui->OSMFileImport->setEnabled(true);
	}
	else
	{
		ui->OSMFileImport->setEnabled(false);
	}
}


void DataTab::saveCurrent()
{
	if (currentIndex_ >= 0 && currentIndex_ < ui->dataSources->count())
	{
		QString name = ui->dataSources->itemText(currentIndex_);

		for (DataSource *output : project_->dataSources())
		{
			if (name == output->userName())
			{
				if (dynamic_cast<OsmDataFile*>(output) != NULL)
				{
					ui->stackedWidget->setCurrentWidget(ui->pageOSMFile);

					OsmDataFile *outputC = dynamic_cast<OsmDataFile*>(output);

					outputC->SetLocalFile(ui->OSMFileName->text());
				}

				break;
			}
		}
	}
}

void DataTab::on_addDataSource_clicked()
{
	// assume a local OSM file for now.
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open OSM File"), "", tr("Image Files (*.osm.pbf *.osm)"));

	OsmDataFile *src = new OsmDataFile();
	src->SetLocalFile(fileName);

	QString primarySourceName = DataSource::primarySourceName();

	bool hasPrimary = false;

	for (DataSource *output : project_->dataSources())
	{
		if (output->dataName() == primarySourceName)
			hasPrimary = true;
	}

	QFileInfo info(fileName);

	QString basename = info.baseName();
	if (basename.length() > 0)
		basename[0] = basename[0].toUpper();

	QString userName = tr("%0 Local Open Street Map File").arg(basename);
	bool nameUsed = false;
	int cycle = 2;
	do
	{
		nameUsed = false;

		for (DataSource *output : project_->dataSources())
		{
			if (QString::compare( output->userName(), userName, Qt::CaseInsensitive) == 0)
				nameUsed = true;
		}

		if (nameUsed)
			userName = tr("%0 %1 Local Open Street Map File").arg(basename).arg(cycle);

		++cycle;
	} 
	while (nameUsed);

	src->setUserName(userName);

	QString dataSourceName;
	if (hasPrimary == false)
	{
		dataSourceName = primarySourceName;
	}
	else
	{
		dataSourceName = info.baseName();
		dataSourceName = dataSourceName.toLower();

		bool nameUsed = false;
		int cycle = 2;
		do
		{
			nameUsed = false;

			for (DataSource *output : project_->dataSources())
			{
				if (QString::compare(output->dataName(), dataSourceName, Qt::CaseInsensitive) == 0)
					nameUsed = true;
			}

			if (nameUsed)
				dataSourceName = QString("%0-%1").arg(info.baseName().toLower()).arg(cycle);

			++cycle;
		} while (nameUsed);
	}

	src->setDataName(dataSourceName);

	project_->addDataSource(src);

	UpdateDataSourceList();
	ui->dataSources->setCurrentIndex(ui->dataSources->count() - 1);
}

void DataTab::on_dataSourceUserRename_clicked()
{
	bool ok = false;

	QString text = QInputDialog::getText(this, tr("Data Source Rename"),
		tr("Data Source Name"), QLineEdit::Normal,
		ui->dataSources->currentText(), &ok);

	int currentIndex = ui->dataSources->currentIndex();

	if (ok && !text.isEmpty())
	{
		for (DataSource *output : project_->dataSources())
		{
			if (ui->dataSources->currentText() == output->userName())
			{
				output->setUserName(text);
				UpdateDataSourceList();
				ui->dataSources->setCurrentIndex(currentIndex);
				break;
			}
		}
	}
}

void DataTab::on_dataSouceIDEdit_clicked()
{
	bool ok = false;

	QString text = QInputDialog::getText(this, tr("Data Source ID Rename"),
		tr("Data Source ID Name"), QLineEdit::Normal,
		ui->dataSourceNameID->text(), &ok);

	if (ok && !text.isEmpty())
	{
		for (DataSource *output : project_->dataSources())
		{
			if (ui->dataSources->currentText() == output->userName())
			{
				output->setDataName(text);
				ui->dataSourceNameID->setText(text);
				break;
			}
		}
	}
}









