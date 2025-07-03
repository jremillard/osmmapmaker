#include <catch2/catch_test_macros.hpp>
#include "output.h"
#include <QtXml>

TEST_CASE("TileOutput default values", "[TileOutput]") {
    TileOutput out("test");
    REQUIRE(out.maxZoom() == 18);
    REQUIRE(out.minZoom() == 13);
    REQUIRE(out.tileSizePixels() == 256);
    REQUIRE(out.resolution1x());
    REQUIRE(out.resolution2x());
    REQUIRE(out.outputDirectory() == "");
}

TEST_CASE("TileOutput setters and getters", "[TileOutput]") {
    TileOutput out("temp");
    out.setMaxZoom(10);
    out.setMinZoom(5);
    out.setTileSizePixels(512);
    out.setResolution1x(false);
    out.setResolution2x(false);
    out.setOutputDirectory("dir");

    REQUIRE(out.maxZoom() == 10);
    REQUIRE(out.minZoom() == 5);
    REQUIRE(out.tileSizePixels() == 512);
    REQUIRE(!out.resolution1x());
    REQUIRE(!out.resolution2x());
    REQUIRE(out.outputDirectory() == "dir");
}

TEST_CASE("TileOutput saveXML populates document", "[TileOutput]") {
    TileOutput out("xml");
    out.setMaxZoom(15);
    out.setMinZoom(8);
    out.setTileSizePixels(512);
    out.setResolution1x(false);
    out.setResolution2x(false);
    out.setOutputDirectory("outdir");

    QDomDocument doc;
    QDomElement elem;
    out.saveXML(doc, elem);

    REQUIRE(elem.tagName() == "tileOutput");
    REQUIRE(elem.attribute("name") == "xml");
    REQUIRE(elem.firstChildElement("maxZoom").text() == "15");
    REQUIRE(elem.firstChildElement("minZoom").text() == "8");
    REQUIRE(elem.firstChildElement("tileSize").text() == "512");
    REQUIRE(elem.firstChildElement("resolution1x").text() == "false");
    REQUIRE(elem.firstChildElement("resolution2x").text() == "false");
    REQUIRE(elem.firstChildElement("directory").text() == "outdir");
}

