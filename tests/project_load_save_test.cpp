#include <catch2/catch_test_macros.hpp>
#include "project.h"
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QStringList>
#include <filesystem>

TEST_CASE("Project load/save round trip is XSD valid", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QXmlSchema schema;
    QFile xsdFile(":/resources/project.xsd");
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());

    QStringList files = {
        QStringLiteral(SOURCE_DIR "/projects/groton-sat.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/projects/groton-trail.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/projects/valid/minimal-file.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/projects/valid/direct-download.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/projects/valid/extract-download.osmmap.xml")
    };

    for (const QString& fileName : files) {
        QFile f(fileName);
        REQUIRE(f.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&f));

        Project project(std::filesystem::path(fileName.toStdString()));
        QTemporaryDir dir;
        std::filesystem::path savePath = std::filesystem::path(dir.path().toStdString()) / "out.osmmap.xml";
        project.save(savePath);

        QFile out(QString::fromStdString(savePath.string()));
        REQUIRE(out.open(QIODevice::ReadOnly));
        REQUIRE(validator.validate(&out));
    }

    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Invalid project XML files fail validation", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QXmlSchema schema;
    QFile xsdFile(":/resources/project.xsd");
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());

    QStringList files = {
        QStringLiteral(SOURCE_DIR "/tests/projects/invalid/missing-map.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/projects/invalid/invalid-tileoutput.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/projects/invalid/missing-datasource-name.osmmap.xml")
    };

    for (const QString& fileName : files) {
        QFile f(fileName);
        REQUIRE(f.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE_FALSE(validator.validate(&f));
    }

    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
