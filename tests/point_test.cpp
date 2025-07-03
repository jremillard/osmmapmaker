#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"
#include <QtXml>

TEST_CASE("Point default values", "[Point]")
{
    Point p;
    REQUIRE(p.visible_);
    REQUIRE(p.color_ == QColor(Qt::black));
    REQUIRE(p.minZoom_ == 0);
    REQUIRE(p.opacity_ == 1);
    REQUIRE(p.width_ == 5);
    REQUIRE(p.image_ == "dot");
}

TEST_CASE("StyleLayer setSubLayerPoint branches", "[Point]")
{
    StyleLayer layer("ds", "k", ST_POINT);
    Point p1;
    p1.width_ = 3;
    p1.image_ = "img1";
    layer.setSubLayerPoint(0, p1);

    REQUIRE(layer.subLayerCount() == 1);
    REQUIRE(layer.subLayerPoint(0).width_ == 3);

    Point p2;
    p2.width_ = 10;
    layer.setSubLayerPoint(0, p2);

    REQUIRE(layer.subLayerCount() == 1);
    REQUIRE(layer.subLayerPoint(0).width_ == 10);

    Point p3;
    p3.width_ = 7;
    layer.setSubLayerPoint(1, p3);
    REQUIRE(layer.subLayerCount() == 2);
    REQUIRE(layer.subLayerPoint(1).width_ == 7);
}

TEST_CASE("StyleLayer save and load point via XML", "[Point]")
{
    StyleLayer layer("ds", "k", ST_POINT);
    Point pt;
    pt.name_ = "pt";
    pt.minZoom_ = 5;
    pt.visible_ = false;
    pt.color_ = QColor(Qt::red);
    pt.width_ = 8;
    pt.opacity_ = 0.75;
    pt.image_ = "circle";
    layer.setSubLayerPoint(0, pt);

    QDomDocument doc;
    QDomElement elem = doc.createElement("layer");
    layer.saveXML(doc, elem);
    doc.appendChild(elem);

    StyleLayer loaded(elem);
    Point loadedPt = loaded.subLayerPoint(0);
    REQUIRE(loadedPt.name_ == pt.name_);
    REQUIRE(loadedPt.minZoom_ == pt.minZoom_);
    REQUIRE(loadedPt.visible_ == pt.visible_);
    REQUIRE(loadedPt.color_ == pt.color_);
    REQUIRE(loadedPt.width_ == pt.width_);
    REQUIRE(loadedPt.opacity_ == pt.opacity_);
    REQUIRE(loadedPt.image_ == pt.image_);
}
