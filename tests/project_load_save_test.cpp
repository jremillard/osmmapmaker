#include <catch2/catch_test_macros.hpp>
#include <QCoreApplication>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include <QNetworkAccessManager>
#include <QEvent>
#include <QStringList>
#include "project.h"
#include <filesystem>

TEST_CASE("Valid project files load and save", "[Project]")
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
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/valid/valid_line.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/valid/valid_point.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/valid/valid_area.osmmap.xml")
    };

    for (const QString& fileName : files) {
        QFile src(fileName);
        REQUIRE(src.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&src));
        src.close();

        std::filesystem::path p = fileName.toStdString();
        {
            Project proj(p);
            for (auto* ds : proj.dataSources()) {
                ds->setImportTime(QDateTime::currentDateTimeUtc());
            }
            std::filesystem::path tmp = p.parent_path() / "tmp_saved.osmmap.xml";
            proj.save(tmp);
            QFile out(QString::fromStdString(tmp.string()));
            REQUIRE(out.open(QIODevice::ReadOnly));
            REQUIRE(validator.validate(&out));
            out.close();
            QFile::remove(QString::fromStdString(tmp.string()));
        }
        std::filesystem::remove_all(p.replace_extension(""));
    }

    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Invalid project files fail validation", "[Project]")
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
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_no_map.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_root.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_missing_layer_attribute.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_tileoutput_boolean.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_two_geometry_tags.osmmap.xml")
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
