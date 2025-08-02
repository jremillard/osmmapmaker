#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "stylelayer.h"
#include <QDomDocument>
#include <QCoreApplication>
#include <QEvent>

TEST_CASE("Area default values", "[Area]")
{
    Area a;
    REQUIRE(a.color_ == QColor(Qt::blue));
    REQUIRE(a.opacity_ == Catch::Approx(1.0));
    REQUIRE(a.casingWidth_ == Catch::Approx(0.0));
    REQUIRE(a.casingColor_ == QColor(Qt::black));
    REQUIRE(a.fillImageOpacity_ == Catch::Approx(1.0));
    REQUIRE(a.fillImage_.isEmpty());
}

TEST_CASE("StyleLayer setSubLayerArea branches", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    StyleLayer layer("ds", "k", ST_AREA);

    Area first;
    first.name_ = "first";
    layer.setSubLayerArea(0, first); // add new
    REQUIRE(layer.subLayerCount() == 1);
    REQUIRE(layer.subLayerArea(0).name_ == "first");

    Area replace;
    replace.name_ = "replace";
    layer.setSubLayerArea(0, replace); // replace existing
    REQUIRE(layer.subLayerCount() == 1);
    REQUIRE(layer.subLayerArea(0).name_ == "replace");

    Area second;
    second.name_ = "second";
    layer.setSubLayerArea(1, second); // append new
    REQUIRE(layer.subLayerCount() == 2);
    REQUIRE(layer.subLayerArea(1).name_ == "second");
}

TEST_CASE("StyleLayer save and load area", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    StyleLayer layer("ds", "k", ST_AREA);
    Area area;
    area.name_ = "area";
    area.minZoom_ = 7;
    area.visible_ = true;
    area.color_ = QColor(Qt::red);
    area.casingColor_ = QColor(Qt::green);
    area.casingWidth_ = 2.0;
    area.opacity_ = 0.5;
    area.fillImage_ = "img";
    area.fillImageOpacity_ = 0.75;

    layer.setSubLayerArea(0, area);

    QDomDocument doc;
    QDomElement elem = doc.createElement("layer");
    layer.saveXML(doc, elem);
    doc.appendChild(elem);

    StyleLayer loaded(elem);
    REQUIRE(loaded.layerType() == ST_AREA);
    Area loadedArea = loaded.subLayerArea(0);
    REQUIRE(loadedArea.name_ == "area");
    REQUIRE(loadedArea.minZoom_ == 7);
    REQUIRE(loadedArea.visible_ == true);
    REQUIRE(loadedArea.color_ == QColor(Qt::red));
    REQUIRE(loadedArea.casingColor_ == QColor(Qt::green));
    REQUIRE(loadedArea.casingWidth_ == Catch::Approx(2.0));
    REQUIRE(loadedArea.opacity_ == Catch::Approx(0.5));
    REQUIRE(loadedArea.fillImage_ == "img");
    REQUIRE(loadedArea.fillImageOpacity_ == Catch::Approx(0.75));
}
