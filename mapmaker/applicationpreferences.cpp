#include "applicationpreferences.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>

QString ApplicationPreferences::appDataDir()
{
    QString env = qEnvironmentVariable("OSMMAPMAKER_APPDATA");
    if (!env.isEmpty()) {
        QDir d(env);
        d.mkpath(".");
        return d.absolutePath();
    }
    QString loc = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) returns
    // a per-user directory suitable for storing persistent application data.
    // The path is never empty unless the location cannot be determined.
    // Typical results:
    //   - Linux:   ~/.local/share/<APPNAME>
    //   - macOS:   ~/Library/Application Support/<APPNAME>
    //   - Windows: C:/Users/<user>/AppData/Roaming/<APPNAME>
    // Set QCoreApplication::setOrganizationName and setApplicationName so
    // <APPNAME> expands correctly.
    QDir d(loc);
    d.mkpath(".");
    return d.absolutePath();
}

std::filesystem::path ApplicationPreferences::mruFilePath()
{
    QString f = appDataDir() + "/recent_projects.txt";
    return std::filesystem::path(f.toStdString());
}

QStringList ApplicationPreferences::readMRU()
{
    QStringList list;
    QFile f(appDataDir() + "/recent_projects.txt");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!f.atEnd()) {
            QString line = QString::fromUtf8(f.readLine()).trimmed();
            if (!line.isEmpty())
                list.append(line);
        }
    }
    return list;
}

void ApplicationPreferences::saveMRU(const QStringList& paths)
{
    QFile f(appDataDir() + "/recent_projects.txt");
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        for (const QString& p : paths) {
            f.write(p.toUtf8());
            f.write("\n");
        }
    }
}

void ApplicationPreferences::addProjectToMRU(const QString& projectPath)
{
    QStringList list = readMRU();
    list.removeAll(projectPath);
    list.prepend(projectPath);
    while (list.size() > ApplicationPreferences::MAX_ENTRIES_MRU)
        list.removeLast();
    saveMRU(list);
}

QString ApplicationPreferences::mostRecentExistingMRU()
{
    QStringList list = readMRU();
    for (const QString& p : list) {
        if (QFile::exists(p))
            return p;
    }
    return QString();
}
