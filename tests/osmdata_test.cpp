#include <catch2/catch_test_macros.hpp>
#include "osmdatafilesource.h"
#include <QtXml>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QDir>
#include <SQLiteCpp/SQLiteCpp.h>
#include "renderdatabase.h"

static const QString baseDir = QStringLiteral(SOURCE_DIR);

TEST_CASE("OsmDataFile setters and getters", "[OsmDataFile]")
{
    OsmDataFileSource file;
    REQUIRE(file.localFile().isEmpty());
    file.SetLocalFile("abc");
    REQUIRE(file.localFile() == "abc");
}

TEST_CASE("DataSource basic properties", "[DataSource]")
{
    OsmDataFileSource file;
    REQUIRE(DataSource::primarySourceName() == "Primary");

    file.setUserName("u");
    file.setDataName("d");
    QDateTime t = QDateTime::currentDateTimeUtc();
    file.setImportTime(t);
    file.setImportDurationS(7);

    REQUIRE(file.userName() == "u");
    REQUIRE(file.dataName() == "d");
    REQUIRE(file.importTime() == t);
    REQUIRE(file.importDurationS() == 7);
}

TEST_CASE("OsmDataFile XML round trip", "[OsmDataFile]")
{
    OsmDataFileSource file;
    file.setUserName("User");
    file.setDataName("Data");
    file.setImportTime(QDateTime::fromString("2023-01-01T00:00:00", Qt::ISODate));
    file.setImportDurationS(42);
    file.SetLocalFile("/tmp/test.osm");

    QDomDocument doc;
    QDomElement elem;
    file.saveXML(doc, elem);

    REQUIRE(elem.tagName() == "openStreetMapFileSource");
    REQUIRE(elem.attribute("name") == "User");
    REQUIRE(elem.firstChildElement("dataSource").text() == "Data");
    REQUIRE(elem.firstChildElement("lastUpdateDate").text() == "2023-01-01T00:00:00");
    REQUIRE(elem.firstChildElement("importDurationS").text() == "42");
    REQUIRE(elem.firstChildElement("fileName").text() == "/tmp/test.osm");

    OsmDataFileSource loaded(elem);
    REQUIRE(loaded.localFile() == "/tmp/test.osm");
    REQUIRE(loaded.userName() == "User");
    REQUIRE(loaded.dataName() == "Data");
}

TEST_CASE("OsmData importFile inserts entities", "[OsmData]")
{
    QString baseDir = QStringLiteral(SOURCE_DIR);
    RenderDatabase db;

    OsmDataFileSource data;
    data.SetLocalFile(baseDir + "/tests/osm/basic.osm");
    data.importData(db);

    SQLite::Statement countStmt(db, "SELECT COUNT(*) FROM entity");
    REQUIRE(countStmt.executeStep());
    int count = countStmt.getColumn(0).getInt();
    REQUIRE(count == 3);

    int typeCounts[3] = { 0, 0, 0 };
    SQLite::Statement typeStmt(db, "SELECT type FROM entity");
    while (typeStmt.executeStep()) {
        int t = typeStmt.getColumn(0).getInt();
        if (t >= 0 && t < 3)
            typeCounts[t]++;
    }
    REQUIRE(typeCounts[OET_POINT] == 1);
    REQUIRE(typeCounts[OET_LINE] == 1);
    REQUIRE(typeCounts[OET_AREA] == 1);
}

TEST_CASE("DataSource cleanDataSource removes data", "[DataSource]")
{
    RenderDatabase db;

    OsmDataFileSource file;

    SQLite::Statement insertEnt(db, "INSERT INTO entity (source, type, geom, linearLengthM, areaM) VALUES (?,?,?,?,?)");
    SQLite::bind(insertEnt, file.dataName().toStdString(), 0, "", 0.0, 0.0);
    insertEnt.exec();
    long long id = db.getLastInsertRowid();

    SQLite::Statement insertIdx(db, "INSERT INTO entitySpatialIndex(pkid,xmin,xmax,ymin,ymax) VALUES (?,?,?,?,?)");
    SQLite::bind(insertIdx, static_cast<int64_t>(id), 0.0, 0.0, 0.0, 0.0);
    insertIdx.exec();

    file.cleanDataSource(db);

    SQLite::Statement countEnt(db, "SELECT COUNT(*) FROM entity");
    REQUIRE(countEnt.executeStep());
    REQUIRE(countEnt.getColumn(0).getInt() == 0);

    SQLite::Statement countIdx(db, "SELECT COUNT(*) FROM entitySpatialIndex");
    REQUIRE(countIdx.executeStep());
    REQUIRE(countIdx.getColumn(0).getInt() == 0);
}

TEST_CASE("Administrative boundary relations import as lines", "[OsmData]")
{
    QString baseDir = QStringLiteral(SOURCE_DIR);
    RenderDatabase db;

    OsmDataFileSource data;
    data.SetLocalFile(baseDir + "/tests/osm/admin_boundary.osm");
    data.importData(db);

    SQLite::Statement countStmt(db, "SELECT COUNT(*) FROM entity WHERE type=?");
    countStmt.bind(1, OET_LINE);
    REQUIRE(countStmt.executeStep());
    REQUIRE(countStmt.getColumn(0).getInt() == 0);
}

TEST_CASE("Rendering sample OSM files import", "[OsmData]")
{
    QString baseDir = QStringLiteral(SOURCE_DIR);
    QDir dir(baseDir + "/tests/osm/render_cases");
    QStringList files = dir.entryList(QStringList() << "case*.osm", QDir::Files);
    REQUIRE(files.size() == 30);

    for (const QString& fileName : files) {
        RenderDatabase rdb;

        OsmDataFileSource data;
        data.SetLocalFile(dir.filePath(fileName));
        data.importData(rdb);

        SQLite::Statement countStmt(rdb, "SELECT COUNT(*) FROM entity");
        REQUIRE(countStmt.executeStep());
        REQUIRE(countStmt.getColumn(0).getInt() > 0);
    }
}

TEST_CASE("Rendering sample OSM schema validation", "[OsmSchema]")
{
    int argc = 0;
    qputenv("QT_PLUGIN_PATH", "");
    qputenv("QT_BEARER_POLL_TIMEOUT", "-1");
    QCoreApplication app(argc, nullptr);
    QCoreApplication::setLibraryPaths(QStringList());

    QXmlSchema schema;
    QFile xsdFile(QStringLiteral(SOURCE_DIR "/tests/osm/osm_test.xsd"));
    REQUIRE(xsdFile.open(QIODevice::ReadOnly));
    schema.load(&xsdFile);
    REQUIRE(schema.isValid());

    QDir dir(QStringLiteral(SOURCE_DIR "/tests/osm/render_cases"));
    QStringList files = dir.entryList(QStringList() << "case*.osm", QDir::Files);
    for (const QString& f : files) {
        QFile dataFile(dir.filePath(f));
        REQUIRE(dataFile.open(QIODevice::ReadOnly));
        QXmlSchemaValidator validator(schema);
        REQUIRE(validator.validate(&dataFile));
    }
    schema = QXmlSchema();
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
