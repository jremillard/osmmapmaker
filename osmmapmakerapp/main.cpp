#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QCommandLineParser>
#include <filesystem>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("OSM Map Maker");
    QCoreApplication::setApplicationVersion("0.2");

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption projectOption(QStringList() << "project", "Project file to open.", "file");
    parser.addOption(projectOption);
    parser.process(a);

    std::filesystem::path projectPath;
    if (parser.isSet(projectOption))
        projectPath = parser.value(projectOption).toStdString();

    try {
        MainWindow w(projectPath);
        w.showMaximized();

        return a.exec();
    } catch (std::exception& e) {
        QMessageBox msgBox;
        msgBox.setText(QString("Internal Error.\n\n%1").arg(e.what()));
        msgBox.exec();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
