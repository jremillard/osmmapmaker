#include "dataTab.h"
#include "ui_dataTab.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <osmdatafile.h>
#include <osmdataoverpass.h>

DataTab::DataTab(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DataTab)
{
    project_ = NULL;
    ui->setupUi(this);
    currentIndex_ = -1;
}

DataTab::~DataTab()
{
    delete ui;
}

void DataTab::setProject(Project* project)
{
    project_ = project;

    UpdateDataSourceList();
}

void DataTab::UpdateDataSourceList()
{
    ui->dataSources->clear();

    if (project_ != NULL) {
        for (DataSource* output : project_->dataSources()) {
            ui->dataSources->addItem(output->userName());
        }
    }

    on_dataSources_currentIndexChanged(ui->dataSources->currentIndex());
}

void DataTab::on_dataSources_currentIndexChanged(int index)
{
    ui->stackedWidget->setCurrentWidget(ui->emptyPage);

    if (currentIndex_ != index) {
        // save it.
        saveCurrent();
    }

    currentIndex_ = index;

    ui->stackedWidget->setCurrentWidget(ui->emptyPage);
    ui->lastImportDate->clear();
    ui->dataSourceNameID->clear();
    ui->dataSouceIDEdit->setEnabled(false);
    ui->dataSourceUserRename->setEnabled(false);

    if (index >= 0) {
        QString name = ui->dataSources->currentText();

        for (DataSource* output : project_->dataSources()) {
            ui->dataSouceIDEdit->setEnabled(true);
            ui->dataSourceUserRename->setEnabled(true);

            if (name == output->userName()) {
                if (auto* fileSrc = dynamic_cast<OsmDataFile*>(output)) {
                    ui->stackedWidget->setCurrentWidget(ui->pageOSMFile);
                    ui->OSMFileName->setText(QDir::toNativeSeparators(fileSrc->localFile()));
                } else if (auto* overSrc = dynamic_cast<OsmDataOverpass*>(output)) {
                    ui->stackedWidget->setCurrentWidget(ui->pageOverpass);
                    ui->overpassQuery->setPlainText(overSrc->query());
                }

                QDateTime importTime = output->importTime();
                QString when;
                if (importTime.isNull()) {
                    ui->lastImportDate->setText(tr("Not Imported Yet"));
                } else {
                    QDateTime now = QDateTime::currentDateTime();

                    long long diffS = importTime.secsTo(now);

                    if (diffS < 60 * 2)
                        when = "Now";
                    else if (diffS < 60 * 60 * 2)
                        when = QString("%1 Minutes Ago").arg(diffS / 60);
                    else if (diffS < 60 * 60 * 24)
                        when = QString("%1 Hours Ago").arg(diffS / (60 * 60));
                    else if (diffS < 60 * 60 * 24 * 15)
                        when = QString("%1 Days Ago").arg(diffS / (60 * 60 * 24));
                    else
                        when = importTime.toString();

                    ui->lastImportDate->setText(tr("%1 - %2 - Duration %3s").arg(when, importTime.toString(), QString::number(output->importDurationS())));
                }

                ui->dataSourceNameID->setText(output->dataName());
            }
        }
    }
}

void DataTab::on_OSMFileDelete_clicked()
{
    QString name = ui->dataSources->currentText();

    for (DataSource* output : project_->dataSources()) {
        if (name == output->userName()) {
            project_->removeDataSource(output);
            UpdateDataSourceList();
            break;
        }
    }
}

void DataTab::on_overpassDelete_clicked()
{
    on_OSMFileDelete_clicked();
}

void DataTab::on_OSMFileBrowse_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open OSM File"), "", tr("Image Files (*.osm.pbf *.osm)"));

    if (file.isEmpty() == false) {
        ui->OSMFileName->setText(QDir::toNativeSeparators(file));
        saveCurrent();
    }
}

void DataTab::on_OSMFileImport_clicked()
{
    QString importName;
    try {
        setCursor(Qt::WaitCursor);

        saveCurrent();

        QString name = ui->dataSources->currentText();

        for (DataSource* output : project_->dataSources()) {
            if (name == output->userName()) {
                RenderDatabase* db = project_->renderDatabase();

                QDateTime startTime = QDateTime::currentDateTime();

                SQLite::Transaction transaction(*db);
                output->cleanDataSource(*db);
                output->importData(*db);
                transaction.commit();

                QDateTime endTime = QDateTime::currentDateTime();

                output->setImportTime(endTime);

                long long int importTimeMS = startTime.secsTo(endTime);

                output->setImportDurationS(importTimeMS);

                on_dataSources_currentIndexChanged(ui->dataSources->currentIndex());

                break;
            }
        }

        setCursor(Qt::ArrowCursor);
    } catch (std::exception& e) {
        this->setCursor(Qt::ArrowCursor);

        QMessageBox msgBox;
        msgBox.setText(QString("Failure importing %1 data.\n%2").arg(importName, e.what()));
        msgBox.exec();
    }
}

void DataTab::on_overpassImport_clicked()
{
    on_OSMFileImport_clicked();
}

void DataTab::on_OSMFileName_textChanged(QString text)
{
    if (QFileInfo::exists(text)) {
        ui->OSMFileImport->setEnabled(true);
    } else {
        ui->OSMFileImport->setEnabled(false);
    }
}

void DataTab::on_overpassQuery_textChanged()
{
    ui->overpassImport->setEnabled(!ui->overpassQuery->toPlainText().isEmpty());
}

void DataTab::saveCurrent()
{
    if (currentIndex_ >= 0 && currentIndex_ < ui->dataSources->count()) {
        QString name = ui->dataSources->itemText(currentIndex_);

        for (DataSource* output : project_->dataSources()) {
            if (name == output->userName()) {
                if (auto* fileSrc = dynamic_cast<OsmDataFile*>(output)) {
                    ui->stackedWidget->setCurrentWidget(ui->pageOSMFile);
                    fileSrc->SetLocalFile(ui->OSMFileName->text());
                } else if (auto* overSrc = dynamic_cast<OsmDataOverpass*>(output)) {
                    ui->stackedWidget->setCurrentWidget(ui->pageOverpass);
                    overSrc->setQuery(ui->overpassQuery->toPlainText());
                }

                break;
            }
        }
    }
}

void DataTab::on_addDataSource_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setText(tr("Add New Map Data Source"));
    QPushButton* fileBtn = msgBox.addButton(tr("Local File"), QMessageBox::AcceptRole);
    QPushButton* overBtn = msgBox.addButton(tr("Overpass"), QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();

    DataSource* src = nullptr;
    QString baseName;
    QString baseId;
    QString fileName;

    if (msgBox.clickedButton() == fileBtn) {
        fileName = QFileDialog::getOpenFileName(this, tr("Open OSM File"), "", tr("Image Files (*.osm.pbf *.osm)"));
        if (fileName.isEmpty())
            return;

        auto* f = new OsmDataFile();
        f->SetLocalFile(fileName);
        src = f;

        QFileInfo info(fileName);
        baseId = info.baseName().toLower();
        baseName = tr("%0 Local Open Street Map File").arg(info.baseName().left(1).toUpper() + info.baseName().mid(1));
    } else if (msgBox.clickedButton() == overBtn) {
        src = new OsmDataOverpass(&networkManager_);
        baseId = "overpass";
        baseName = tr("Overpass Data Source");
    } else {
        return;
    }

    QString primarySourceName = DataSource::primarySourceName();
    bool hasPrimary = false;
    for (DataSource* output : project_->dataSources()) {
        if (output->dataName() == primarySourceName)
            hasPrimary = true;
    }

    QString userName = baseName;
    bool nameUsed = false;
    int cycle = 2;
    do {
        nameUsed = false;
        for (DataSource* output : project_->dataSources()) {
            if (QString::compare(output->userName(), userName, Qt::CaseInsensitive) == 0)
                nameUsed = true;
        }
        if (nameUsed)
            userName = tr("%0 %1").arg(baseName).arg(cycle);
        ++cycle;
    } while (nameUsed);
    src->setUserName(userName);

    QString dataSourceName;
    if (!hasPrimary) {
        dataSourceName = primarySourceName;
    } else {
        dataSourceName = baseId;
        bool used = false;
        int cycle = 2;
        do {
            used = false;
            for (DataSource* output : project_->dataSources()) {
                if (QString::compare(output->dataName(), dataSourceName, Qt::CaseInsensitive) == 0)
                    used = true;
            }
            if (used)
                dataSourceName = QString("%0-%1").arg(baseId).arg(cycle);
            ++cycle;
        } while (used);
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

    if (ok && !text.isEmpty()) {
        for (DataSource* output : project_->dataSources()) {
            if (ui->dataSources->currentText() == output->userName()) {
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

    if (ok && !text.isEmpty()) {
        for (DataSource* output : project_->dataSources()) {
            if (ui->dataSources->currentText() == output->userName()) {
                output->setDataName(text);
                ui->dataSourceNameID->setText(text);
                break;
            }
        }
    }
}
