#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QCommandLineParser>
#include <filesystem>
#include "project.h"
#include "batchtileoutput.h"

struct OutputOption {
    bool requested = false;
    QString name;
};

static OutputOption parseOutputOption(QStringList& args)
{
    OutputOption opt;
    for (int i = 1; i < args.size(); ++i) {
        QString arg = args.at(i);
        if (arg == "--output") {
            opt.requested = true;
            args.removeAt(i);
            if (i < args.size() && !args.at(i).startsWith('-')) {
                opt.name = args.at(i);
                args.removeAt(i);
            }
            break;
        } else if (arg.startsWith("--output=")) {
            opt.requested = true;
            opt.name = arg.mid(QString("--output=").size());
            args.removeAt(i);
            break;
        }
    }
    return opt;
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("OSM Map Maker");
    QCoreApplication::setApplicationVersion("0.2");

    QStringList args = a.arguments();
    OutputOption outputOpt = parseOutputOption(args);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption projectOption(QStringList() << "project", "Project file to open.", "file");
    parser.addOption(projectOption);
    parser.process(args);

    std::filesystem::path projectPath;
    if (parser.isSet(projectOption))
        projectPath = parser.value(projectOption).toStdString();

    try {
        if (outputOpt.requested) {
            if (projectPath.empty())
                throw std::runtime_error("--output requires --project <file>");

            Project project(projectPath);

            if (outputOpt.name.isEmpty()) {
                for (Output* out : project.outputs()) {
                    if (auto* t = dynamic_cast<TileOutput*>(out))
                        BatchTileOutput::generateTiles(&project, *t);
                }
            } else {
                bool found = false;
                for (Output* out : project.outputs()) {
                    if (out->name() == outputOpt.name) {
                        if (auto* t = dynamic_cast<TileOutput*>(out)) {
                            BatchTileOutput::generateTiles(&project, *t);
                            found = true;
                        }
                    }
                }
                if (!found)
                    throw std::runtime_error("No output named " + outputOpt.name.toStdString());
            }
            return EXIT_SUCCESS;
        }

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
