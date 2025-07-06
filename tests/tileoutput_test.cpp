#include <catch2/catch_test_macros.hpp>
#include "tileoutput.h"
#include <QtXml>

TEST_CASE("TileOutput default values", "[TileOutput]")
{
    TileOutput out("test");
    REQUIRE(out.maxZoom() == 18);
    REQUIRE(out.minZoom() == 13);
    REQUIRE(out.tileSizePixels() == 256);
    REQUIRE(out.resolution1x());
    REQUIRE(out.resolution2x());
    REQUIRE(out.outputDirectory() == "");
}

TEST_CASE("TileOutput setters and getters", "[TileOutput]")
{
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

TEST_CASE("TileOutput saveXML populates document", "[TileOutput]")
{
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

TEST_CASE("Output XML serialization and loading", "[Output]")
{
    QDomDocument doc;
    QDomElement elem = doc.createElement("output");
    elem.setAttribute("name", "base");
    Output base(elem);
    REQUIRE(base.name() == "base");
    base.setName("changed");

    QDomDocument saveDoc;
    QDomElement saveElem = saveDoc.createElement("output");
    base.saveXML(saveDoc, saveElem);
    REQUIRE(saveElem.attribute("name") == "changed");
}

TEST_CASE("TileOutput constructed from DOM", "[TileOutput]")
{
    QDomDocument doc;
    QDomElement elem = doc.createElement("tileOutput");
    elem.setAttribute("name", "dom");

    auto addChild = [&](const QString& n, const QString& t) {
        QDomElement child = doc.createElement(n);
        child.appendChild(doc.createTextNode(t));
        elem.appendChild(child);
    };
    addChild("maxZoom", "20");
    addChild("minZoom", "5");
    addChild("tileSize", "128");
    addChild("resolution1x", "false");
    addChild("resolution2x", "true");
    addChild("directory", "out");

    TileOutput loaded(elem);
    REQUIRE(loaded.name() == "dom");
    REQUIRE(loaded.maxZoom() == 20);
    REQUIRE(loaded.minZoom() == 5);
    REQUIRE(loaded.tileSizePixels() == 128);
    REQUIRE(!loaded.resolution1x());
    REQUIRE(loaded.resolution2x());
    REQUIRE(loaded.outputDirectory() == "out");
}
