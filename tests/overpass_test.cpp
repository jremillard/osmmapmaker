#include <catch2/catch_test_macros.hpp>
#include <QNetworkAccessManager>
#include <QtXml>
#include <SQLiteCpp/SQLiteCpp.h>
#include "osmdataoverpass.h"
#include "renderdatabase.h"

TEST_CASE("Overpass import uses cache", "[Overpass]")
{
    RenderDatabase db;

    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);

    QString query = "node(0,0,1,1);out;";
    QByteArray osm = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><osm version=\"0.6\"><node id=\"1\" lat=\"0\" lon=\"0\"><tag k=\"name\" v=\"n1\"/></node></osm>";
    OsmDataOverpass::cache_.insert(query, osm);

    OsmDataOverpass over(&nam);
    over.setQuery(query);
    over.importData(db);

    SQLite::Statement countStmt(db, "SELECT COUNT(*) FROM entity");
    REQUIRE(countStmt.executeStep());
    REQUIRE(countStmt.getColumn(0).getInt() == 1);
}

TEST_CASE("Overpass network failure throws", "[Overpass]")
{
    QNetworkAccessManager nam;
    nam.setNetworkAccessible(QNetworkAccessManager::NotAccessible);
    OsmDataOverpass over(&nam);
    over.setQuery("foo");

    RenderDatabase db;

    REQUIRE_THROWS(over.importData(db));
}
