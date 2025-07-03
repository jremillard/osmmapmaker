#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"
#include <QCoreApplication>
#include <QEvent>

TEST_CASE("StyleLayer dataType and virtual table", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layerP("ds", "k", ST_POINT);
    REQUIRE(layerP.dataType() == OET_POINT);
    REQUIRE(layerP.virtualSQLTableName() == "k_ds_point_v");

    StyleLayer layerL("ds", "k", ST_LINE);
    REQUIRE(layerL.dataType() == OET_LINE);
    REQUIRE(layerL.virtualSQLTableName() == "k_ds_line_v");

    StyleLayer layerA("ds", "k", ST_AREA);
    REQUIRE(layerA.dataType() == OET_AREA);
    REQUIRE(layerA.virtualSQLTableName() == "k_ds_area_v");

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer show and hide", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "k", ST_POINT);
    Point pt;
    layer.setSubLayerPoint(0, pt);
    layer.setSubLayerPoint(1, pt);

    layer.hideAll();
    REQUIRE(!layer.subLayerPoint(0).visible_);
    REQUIRE(!layer.subLayerPoint(1).visible_);

    layer.showAll();
    REQUIRE(layer.subLayerPoint(0).visible_);
    REQUIRE(layer.subLayerPoint(1).visible_);

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer move and remove", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "k", ST_POINT);
    Point p1;
    p1.name_ = "one";
    Point p2;
    p2.name_ = "two";
    layer.setSubLayerPoint(0, p1);
    layer.setSubLayerPoint(1, p2);

    layer.subLayerMove(0, 1);
    REQUIRE(layer.subLayerPoint(1).name_ == "one");

    layer.removeSubLayer(1);
    REQUIRE(layer.subLayerCount() == 1);

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer required keys", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "primary", ST_POINT);
    Point pt;
    layer.setSubLayerPoint(0, pt);
    layer.setSubLayerPoint(1, pt);

    Label lb = layer.label(0);
    lb.text_ = "[name]";
    layer.setLabel(0, lb);

    Label lb2 = layer.label(1);
    lb2.visible_ = false;
    layer.setLabel(1, lb2);

    StyleSelector sel0 = layer.subLayerSelectors(0);
    sel0.insertCondition(1, "extra", { "yes" });
    layer.setSubLayerSelectors(0, sel0);

    StyleSelector sel1 = layer.subLayerSelectors(1);
    sel1.insertCondition(1, "primary", { "*" });
    sel1.insertCondition(2, "dup", { "val" });
    layer.setSubLayerSelectors(1, sel1);

    auto keys = layer.requiredKeys();
    REQUIRE(keys.size() == 3);
    REQUIRE(std::find(keys.begin(), keys.end(), "name") != keys.end());
    REQUIRE(std::find(keys.begin(), keys.end(), "extra") != keys.end());
    REQUIRE(std::find(keys.begin(), keys.end(), "dup") != keys.end());
    REQUIRE(std::find(keys.begin(), keys.end(), "primary") == keys.end());

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer sub layer names", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "k", ST_POINT);
    Point pt;
    layer.setSubLayerPoint(0, pt);
    layer.setSubLayerPoint(1, pt);
    layer.setSubLayerPoint(2, pt);

    StyleSelector s0 = layer.subLayerSelectors(0);
    s0.setCondition(0, "k", { QString(40, 'a') });
    layer.setSubLayerSelectors(0, s0);

    StyleSelector s1 = layer.subLayerSelectors(1);
    s1.setCondition(0, "k", { "foo" });
    s1.insertCondition(1, "other", { "bar" });
    layer.setSubLayerSelectors(1, s1);

    StyleSelector s2 = layer.subLayerSelectors(2);
    s2.setCondition(0, "k", { "*" });
    layer.setSubLayerSelectors(2, s2);

    auto names = layer.subLayerNames();
    REQUIRE(names[0].endsWith("..."));
    REQUIRE(names[1] == "foo, other=");
    REQUIRE(names[2] == "All");

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
