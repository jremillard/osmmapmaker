#include <catch2/catch_test_macros.hpp>
#include "osmdatafile.h"
#include <QtXml>
#include <SQLiteCpp/SQLiteCpp.h>
#include <fstream>
#include <sstream>

static void execSqlFile(SQLite::Database& db, const std::string& path)
{
    std::ifstream in(path);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    db.exec(ss.str());
}

TEST_CASE("OsmDataFile setters and getters", "[OsmDataFile]")
{
    OsmDataFile file;
    REQUIRE(file.localFile().isEmpty());
    file.SetLocalFile("abc");
    REQUIRE(file.localFile() == "abc");
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
    QString baseDir = QStringLiteral(SOURCE_DIR);
    SQLite::Database db(":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    execSqlFile(db, (baseDir + "/osmmapmakerapp/resources/render-0.sql").toStdString());
    execSqlFile(db, (baseDir + "/osmmapmakerapp/resources/render-1.sql").toStdString());

    QTemporaryFile osmTemp("XXXXXX.osm");
    REQUIRE(osmTemp.open());
    QString osmContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                         "<osm version=\"0.6\" generator=\"test\">\n"
                         "  <node id=\"1\" lat=\"0.0\" lon=\"0.0\"><tag k=\"name\" v=\"n1\"/></node>\n"
                         "  <node id=\"2\" lat=\"0.0\" lon=\"1.0\"/>\n"
                         "  <node id=\"3\" lat=\"1.0\" lon=\"1.0\"/>\n"
                         "  <node id=\"4\" lat=\"1.0\" lon=\"0.0\"/>\n"
                         "  <way id=\"10\">\n"
                         "    <nd ref=\"1\"/><nd ref=\"2\"/><nd ref=\"3\"/><nd ref=\"4\"/><nd ref=\"1\"/>\n"
                         "    <tag k=\"area\" v=\"yes\"/>\n"
                         "    <tag k=\"name\" v=\"square\"/>\n"
                         "  </way>\n"
                         "  <way id=\"11\">\n"
                         "    <nd ref=\"1\"/><nd ref=\"2\"/><nd ref=\"3\"/>\n"
                         "    <tag k=\"highway\" v=\"residential\"/>\n"
                         "    <tag k=\"area\" v=\"no\"/>\n"
                         "  </way>\n"
                         "</osm>\n";
    osmTemp.write(osmContent.toUtf8());
    osmTemp.close();

    OsmDataFile data;
    data.SetLocalFile(osmTemp.fileName());
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
