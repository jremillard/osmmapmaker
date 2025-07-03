#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"
#include <QtCore>

TEST_CASE("SubLayer base defaults", "[SubLayer]")
{
    SubLayer s;
    REQUIRE(s.visible_ == true);
    REQUIRE(s.color_ == QColor(Qt::black));
    REQUIRE(s.minZoom_ == 0);
}

TEST_CASE("Point defaults", "[SubLayer]")
{
    Point p;
    REQUIRE(p.visible_ == true);
    REQUIRE(p.color_ == QColor(Qt::black));
    REQUIRE(p.minZoom_ == 0);
    REQUIRE(p.opacity_ == 1.0);
    REQUIRE(p.width_ == 5.0);
    REQUIRE(p.image_ == "dot");
}

TEST_CASE("Line defaults", "[SubLayer]")
{
    Line l;
    REQUIRE(l.visible_ == true);
    REQUIRE(l.color_ == QColor(Qt::black));
    REQUIRE(l.minZoom_ == 0);
    REQUIRE(l.opacity_ == 1.0);
    REQUIRE(l.width_ == 1.0);
    REQUIRE(l.casingWidth_ == 0.0);
    REQUIRE(l.casingColor_ == QColor(Qt::white));
    REQUIRE(l.smooth_ == 0.0);
}

TEST_CASE("Area defaults", "[SubLayer]")
{
    Area a;
    REQUIRE(a.visible_ == true);
    REQUIRE(a.color_ == QColor(Qt::blue));
    REQUIRE(a.minZoom_ == 0);
    REQUIRE(a.opacity_ == 1.0);
    REQUIRE(a.casingWidth_ == 0.0);
    REQUIRE(a.casingColor_ == QColor(Qt::black));
    REQUIRE(a.fillImageOpacity_ == 1.0);
}

TEST_CASE("Label defaults and mapnikText", "[SubLayer]")
{
    Label lbl;
    REQUIRE(lbl.visible_ == true);
    REQUIRE(lbl.color_ == QColor(Qt::black));
    REQUIRE(lbl.minZoom_ == 0);
    REQUIRE(lbl.text_ == "[name]");
    REQUIRE(lbl.height_ == 10.0);
    REQUIRE(lbl.haloSize_ == 0.0);
    REQUIRE(lbl.haloColor_ == QColor(Qt::black));
    REQUIRE(lbl.maxWrapWidth_ == 30.0);
    REQUIRE(lbl.offsetY_ == 0.0);
    REQUIRE(lbl.fontWeight == 400);
    REQUIRE(lbl.priority_ == 0);
    REQUIRE(lbl.mapnikText() == "[name]");
}

TEST_CASE("StyleLayer setSubLayer methods add and replace", "[SubLayer]")
{
    StyleLayer layerPt("ds", "k", ST_POINT);
    Point p1;
    p1.width_ = 3.0;
    layerPt.setSubLayerPoint(0, p1);
    REQUIRE(layerPt.subLayerCount() == 1);
    REQUIRE(layerPt.subLayerPoint(0).width_ == 3.0);
    Point p2;
    p2.width_ = 6.0;
    layerPt.setSubLayerPoint(0, p2);
    REQUIRE(layerPt.subLayerCount() == 1);
    REQUIRE(layerPt.subLayerPoint(0).width_ == 6.0);

    StyleLayer layerLn("ds", "k", ST_LINE);
    Line l1;
    l1.width_ = 1.0;
    layerLn.setSubLayerLine(0, l1);
    Line l2;
    l2.width_ = 2.0;
    layerLn.setSubLayerLine(1, l2);
    REQUIRE(layerLn.subLayerCount() == 2);
    layerLn.subLayerMove(0, 1);
    REQUIRE(layerLn.subLayerLine(1).width_ == 1.0);
    layerLn.removeSubLayer(0);
    REQUIRE(layerLn.subLayerCount() == 1);
    REQUIRE(layerLn.subLayerLine(0).width_ == 1.0);

    StyleLayer layerAr("ds", "k", ST_AREA);
    Area a;
    layerAr.setSubLayerArea(0, a);
    layerAr.hideAll();
    REQUIRE(layerAr.subLayerArea(0).visible_ == false);
    layerAr.showAll();
    REQUIRE(layerAr.subLayerArea(0).visible_ == true);
}

TEST_CASE("StyleLayer virtualSQLTableName", "[SubLayer]")
{
    StyleLayer lp("ds", "k", ST_POINT);
    REQUIRE(lp.virtualSQLTableName() == "k_ds_point_v");
    StyleLayer ll("ds", "k", ST_LINE);
    REQUIRE(ll.virtualSQLTableName() == "k_ds_line_v");
    StyleLayer la("ds", "k", ST_AREA);
    REQUIRE(la.virtualSQLTableName() == "k_ds_area_v");
}
