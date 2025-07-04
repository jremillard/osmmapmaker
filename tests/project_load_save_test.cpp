#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/catch_approx.hpp>
#include <QCoreApplication>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QFile>
#include <QNetworkAccessManager>
#include <QEvent>
#include <QStringList>
#include "project.h"
#include "osmdatafile.h"
#include "stylelayer.h"
#include "output.h"
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
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_two_geometry_tags.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_field_name.osmmap.xml")
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

TEST_CASE("Invalid project files throw on load", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QStringList files = {
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_no_map.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_root.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_missing_layer_attribute.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_tileoutput_boolean.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_two_geometry_tags.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_negative_zoom.osmmap.xml"),
        QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/invalid/invalid_bad_field_name.osmmap.xml")
    };

    for (const QString& fileName : files) {
        INFO(fileName.toStdString());
        std::filesystem::path p = fileName.toStdString();
        try {
            Project proj(p);
            FAIL("Expected exception not thrown");
        } catch (const std::runtime_error& e) {
            std::string msg = e.what();
            CHECK_THAT(msg, Catch::Matchers::ContainsSubstring(fileName.toStdString()));
            CHECK_THAT(msg, Catch::Matchers::Matches(".*:\\d+:.*"));
        }
    }

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Malformed project file reports parse location", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QString fileName = QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/malformed/malformed_missing_lt.osmmap.xml");
    std::filesystem::path p = fileName.toStdString();

    try {
        Project proj(p);
        FAIL("Expected exception not thrown");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        CHECK_THAT(msg, Catch::Matchers::ContainsSubstring(fileName.toStdString()));
        CHECK_THAT(msg, Catch::Matchers::Matches(".*:\\d+:.*"));
    }

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Project coordinate conversions", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QString fileName = QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/valid/valid_point.osmmap.xml");
    std::filesystem::path p = fileName.toStdString();

    {
        Project proj(p);

        REQUIRE(proj.mapSRS().find("merc") != std::string::npos);
        REQUIRE(proj.dataSRS().find("longlat") != std::string::npos);

        double x = 0.0;
        double y = 0.0;
        proj.convertDataToMap(1.0, 2.0, &x, &y);
        double lon = 0.0;
        double lat = 0.0;
        proj.convertMapToData(x, y, &lon, &lat);

        REQUIRE(lon == Catch::Approx(1.0).margin(1e-6));
        REQUIRE(lat == Catch::Approx(2.0).margin(1e-6));

        REQUIRE(proj.projectPath() == p);
        REQUIRE(std::filesystem::exists(proj.assetDirectory()));
    }

    std::filesystem::remove_all(p.replace_extension(""));

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Project add and remove components", "[Project]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QString fileName = QStringLiteral(SOURCE_DIR "/tests/project_xml_samples/valid/valid_point.osmmap.xml");
    std::filesystem::path p = fileName.toStdString();

    {
        Project proj(p);
        size_t dataCount = proj.dataSources().size();
        size_t outCount = proj.outputs().size();
        size_t layerCount = proj.styleLayers().size();

        auto* ds = new OsmDataFile();
        proj.addDataSource(ds);
        REQUIRE(proj.dataSources().size() == dataCount + 1);
        proj.removeDataSource(ds);
        REQUIRE(proj.dataSources().size() == dataCount);

        auto* out = new TileOutput("extra");
        proj.addOutput(out);
        REQUIRE(proj.outputs().size() == outCount + 1);
        proj.removeOutput(out);
        REQUIRE(proj.outputs().size() == outCount);

        auto* layer = new StyleLayer("Primary", "amenity", ST_POINT);
        proj.addStyleLayer(proj.styleLayers().size(), layer);
        REQUIRE(proj.styleLayers().size() == layerCount + 1);
        proj.removeStyleLayer(layer);
        REQUIRE(proj.styleLayers().size() == layerCount);

        QColor origColor = proj.backgroundColor();
        proj.setBackgroundColor(Qt::red);
        REQUIRE(proj.backgroundColor() == QColor(Qt::red));
        proj.setBackgroundColor(origColor);

        double origOp = proj.backgroundOpacity();
        proj.setBackgroundOpacity(0.25);
        REQUIRE(proj.backgroundOpacity() == Catch::Approx(0.25));
        proj.setBackgroundOpacity(origOp);
    }

    std::filesystem::remove_all(p.replace_extension(""));
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
