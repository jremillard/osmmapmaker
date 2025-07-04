#include <catch2/catch_test_macros.hpp>
#include "osmdatafile.h"
#include "osmdatadirectdownload.h"
#include "osmdataextractdownload.h"
#include <QtXml>
#include <SQLiteCpp/SQLiteCpp.h>
#include "renderdatabase.h"

TEST_CASE("OsmDataFile setters and getters", "[OsmDataFile]")
{
    OsmDataFile file;
    REQUIRE(file.localFile().isEmpty());
    file.SetLocalFile("abc");
    REQUIRE(file.localFile() == "abc");
}

TEST_CASE("DataSource basic properties", "[DataSource]")
{
    OsmDataFile file;
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
    OsmDataFile file;
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

    OsmDataFile loaded(elem);
    REQUIRE(loaded.localFile() == "/tmp/test.osm");
    REQUIRE(loaded.userName() == "User");
    REQUIRE(loaded.dataName() == "Data");
}

TEST_CASE("OsmData importFile inserts entities", "[OsmData]")
{
    RenderDatabase rdb;
    SQLite::Database& db = rdb.db();

    OsmDataFile data;
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
    RenderDatabase rdb;
    SQLite::Database& db = rdb.db();

    OsmDataFile file;

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
    SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    execSqlFile(db, (baseDir + "/osmmapmakerapp/resources/render-0.sql").toStdString());
    execSqlFile(db, (baseDir + "/osmmapmakerapp/resources/render-1.sql").toStdString());

    OsmDataFile data;
    data.SetLocalFile(baseDir + "/tests/osm/admin_boundary.osm");
    data.importData(db);

    SQLite::Statement countStmt(db, "SELECT COUNT(*) FROM entity WHERE type=?");
    countStmt.bind(1, OET_LINE);
    REQUIRE(countStmt.executeStep());
    REQUIRE(countStmt.getColumn(0).getInt() == 1);

    SQLite::Statement tagStmt(db, "SELECT value FROM entityKV WHERE key='admin_level'");
    REQUIRE(tagStmt.executeStep());
    REQUIRE(tagStmt.getColumn(0).getString() == std::string("8"));
}

TEST_CASE("OsmDataDirectDownload saveXML", "[OsmData]")
{
    QDomDocument doc;
    QDomElement elem = doc.createElement("openStreetMapDirectDownload");
    OsmDataDirectDownload dl(elem);

    QDomDocument outDoc;
    QDomElement outElem;
    dl.saveXML(outDoc, outElem);

    REQUIRE(outElem.tagName() == "openStreetMapDirectDownload");
    REQUIRE(outElem.firstChildElement("dataSource").text() == "OSM");
}

TEST_CASE("OsmDataExtractDownload saveXML", "[OsmData]")
{
    QDomDocument doc;
    QDomElement elem = doc.createElement("openStreetMapExtractDownload");
    OsmDataExtractDownload ex(elem);

    QDomDocument outDoc;
    QDomElement outElem;
    ex.saveXML(outDoc, outElem);

    REQUIRE(outElem.tagName() == "openStreetMapExtractDownload");
    REQUIRE(outElem.firstChildElement("dataSource").text() == "OSM");
}
