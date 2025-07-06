#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dataTab.h"
#include "styleTab.h"
#include "outputTab.h"

#include "project.h"
#include "newprojectdialog.h"
#include "projecttemplate.h"
#include "applicationpreferences.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <filesystem>
#include <QFile>
#include <QCoreApplication>

MainWindow::MainWindow(path projectPath, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->addTab(new DataTab(this), tr("Data"));
    ui->tabWidget->addTab(new StyleTab(this), tr("Style"));
    ui->tabWidget->addTab(new OutputTab(this), tr("Output"));
    recentMenu_ = ui->menuRecentProjects;
    updateRecentMenu();

    project_ = NULL;

    bool opened = false;
    if (!projectPath.empty()) {
        try {
            openProject(projectPath);
            opened = true;
        } catch (std::exception&) {
        }
    }

    if (!opened) {
        QString recent = ApplicationPreferences::mostRecentExistingMRU();
        if (!recent.isEmpty()) {
            try {
                openProject(recent.toStdString());
                opened = true;
            } catch (std::exception&) {
            }
        }
    }

    if (!opened) {
        auto locs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        QString loc;
        if (locs.size() > 0)
            loc = locs[0];

        QString file = QFileDialog::getOpenFileName(this, tr("Open Project"), loc, tr("Map Project Files (*.xml)"));

        if (file.isEmpty()) {
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
    setWindowTitle(QString("OSM Map Maker %1 - %2")
            .arg(QCoreApplication::applicationVersion(),
                QString::fromStdWString(projectPath.filename().wstring())));

    DataTab* dataTab = dynamic_cast<DataTab*>(ui->tabWidget->widget(0));
    dataTab->setProject(project_);

    StyleTab* styleTab = dynamic_cast<StyleTab*>(ui->tabWidget->widget(1));
    styleTab->setProject(project_);

    OutputTab* outputTab = dynamic_cast<OutputTab*>(ui->tabWidget->widget(2));
    outputTab->setProject(project_);

    if (project_->dataSources().size() == 0) {
        ui->tabWidget->setCurrentIndex(0);
    } else {
        ui->tabWidget->setCurrentIndex(1);
    }

    ApplicationPreferences::addProjectToMRU(QString::fromStdWString(projectPath.wstring()));
    updateRecentMenu();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete project_;
}

void MainWindow::on_action_Project_New_triggered()
{
    NewProjectDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QString path = dlg.projectPath();
        if (path.isEmpty())
            return;

        std::filesystem::path p = path.toStdString();
        QString name = QString::fromStdString(p.stem().string());
        std::filesystem::path dir = p.parent_path();

        try {
            QByteArray bytes = ProjectTemplate::projectTemplateContents(dlg.templateName());
            Project::createNew(name, dir, bytes);
        } catch (const std::exception& e) {
            QMessageBox::warning(this, tr("New Project"), e.what());
            return;
        }

        try {
            openProject(path.toStdString());
        } catch (std::exception& e) {
            QMessageBox::warning(this, tr("New Project"), e.what());
        }
    }
}

void MainWindow::on_action_Project_Open_triggered()
{
    auto locs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString loc;
    if (locs.size() > 0)
        loc = locs[0];

    QString file = QFileDialog::getOpenFileName(this, tr("Open Project"), loc, tr("Map Project Files (*.xml)"));

    if (file.isEmpty() == false) {
        openProject(file.toStdString());
    }
}

void MainWindow::on_action_Project_Copy_triggered()
{
    if (project_ == NULL)
        return;

    auto locs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString loc;
    if (!locs.isEmpty())
        loc = locs[0];

    QString defaultName = QString::fromStdString(project_->projectPath().filename().string());
    QString filter = tr("Map Project Files (*%1)").arg(Project::projectFileExtension());
    QString file = QFileDialog::getSaveFileName(this, tr("Copy Project"), loc + QDir::separator() + defaultName, filter);
    if (file.isEmpty())
        return;

    std::filesystem::path dest = std::filesystem::path(file.toStdString());
    if (std::filesystem::exists(dest) || std::filesystem::exists(dest.replace_extension(""))) {
        QMessageBox::warning(this, tr("Copy Project"), tr("Destination already exists."));
        return;
    }

    try {
        project_->saveTo(dest);
        QMessageBox::information(this, tr("Copy Project"),
            QString("Copied to %1").arg(QString::fromStdString(dest.string())));
    } catch (const std::exception& e) {
        QMessageBox msgBox(this);
        msgBox.setText(QString("Failed copy: %1").arg(e.what()));
        msgBox.exec();
    }
}

void MainWindow::on_action_Project_Save_triggered()
{
    try {
        setCursor(Qt::WaitCursor);

        if (project_ != NULL)
            project_->save();

        setCursor(Qt::ArrowCursor);
    } catch (std::exception& e) {
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

void MainWindow::openRecentProject()
{
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    QString path = a->data().toString();
    if (!path.isEmpty())
        openProject(path.toStdString());
}

void MainWindow::updateRecentMenu()
{
    recentMenu_->clear();
    QStringList list = ApplicationPreferences::readMRU();
    int added = 0;
    for (const QString& p : list) {
        if (!QFile::exists(p))
            continue;
        QAction* act = recentMenu_->addAction(p);
        act->setData(p);
        connect(act, &QAction::triggered, this, &MainWindow::openRecentProject);
        if (++added >= MAX_MENU_RECENT)
            break;
    }
    recentMenu_->setEnabled(added > 0);
}
