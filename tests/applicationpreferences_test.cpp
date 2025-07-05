#include <catch2/catch_test_macros.hpp>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QTemporaryFile>
#include "applicationpreferences.h"

TEST_CASE("Recent project list persists", "[ApplicationPreferences]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    QTemporaryDir dir;
    qputenv("OSMMAPMAKER_APPDATA", dir.path().toUtf8());

    ApplicationPreferences::addProjectToMRU("/tmp/proj1.osmmap.xml");
    ApplicationPreferences::addProjectToMRU("/tmp/proj2.osmmap.xml");
    ApplicationPreferences::addProjectToMRU("/tmp/proj3.osmmap.xml");

    QStringList list = ApplicationPreferences::readMRU();
    REQUIRE(list.size() == 3);
    REQUIRE(list.first() == "/tmp/proj3.osmmap.xml");

    // Simulate new session
    QStringList list2 = ApplicationPreferences::readMRU();
    REQUIRE(list2 == list);
}

TEST_CASE("Most recent existing project selected", "[ApplicationPreferences]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    QTemporaryDir dir;
    qputenv("OSMMAPMAKER_APPDATA", dir.path().toUtf8());

    QTemporaryFile f(dir.path() + "/proj.osmmap.xml");
    REQUIRE(f.open());
    QString p = f.fileName();

    ApplicationPreferences::addProjectToMRU("/does/not/exist.osmmap.xml");
    ApplicationPreferences::addProjectToMRU(p);

    QString recent = ApplicationPreferences::mostRecentExistingMRU();
    REQUIRE(recent == p);
}

TEST_CASE("Utility functions return expected paths", "[ApplicationPreferences]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    QTemporaryDir dir;
    qputenv("OSMMAPMAKER_APPDATA", dir.path().toUtf8());

    QString ad = ApplicationPreferences::appDataDir();
    REQUIRE(ad == dir.path());

    auto mru = ApplicationPreferences::mruFilePath();
    REQUIRE(QString::fromStdString(mru.parent_path().string()) == dir.path());

    QStringList paths;
    paths << "/tmp/proj1.xml";
    ApplicationPreferences::saveMRU(paths);
    QStringList loaded = ApplicationPreferences::readMRU();
    REQUIRE(loaded == paths);
}
