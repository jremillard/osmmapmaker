#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
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

TEST_CASE("StyleLayer renderSQLSelect includes joins", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "highway", ST_LINE);
    Line ln;
    layer.setSubLayerLine(0, ln);

    Label lb = layer.label(0);
    lb.text_ = "[name]";
    layer.setLabel(0, lb);

    StyleSelector sel = layer.subLayerSelectors(0);
    sel.insertCondition(1, "access", { "private" });
    layer.setSubLayerSelectors(0, sel);

    QString sql = layer.renderSQLSelect(false);
    REQUIRE(sql.contains("\"highway\".value as \"highway\""));
    REQUIRE(sql.contains("\"name\".value as \"name\""));
    REQUIRE(sql.contains("left outer join entityKV as \"access\""));

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer save and load point", "[StyleLayer]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);

    StyleLayer layer("ds", "k", ST_POINT);
    Point pt;
    pt.name_ = "pt";
    pt.minZoom_ = 2;
    pt.color_ = QColor(Qt::green);
    pt.opacity_ = 0.5;
    pt.image_ = "img";
    pt.width_ = 7;
    layer.setSubLayerPoint(0, pt);

    Label lb;
    lb.visible_ = true;
    lb.text_ = "[name]";
    lb.color_ = QColor(Qt::yellow);
    lb.height_ = 8;
    layer.setLabel(0, lb);

    QDomDocument doc;
    QDomElement elem = doc.createElement("layer");
    layer.saveXML(doc, elem);
    doc.appendChild(elem);

    StyleLayer loaded(elem);
    REQUIRE(loaded.layerType() == ST_POINT);
    Point loadedPt = loaded.subLayerPoint(0);
    REQUIRE(loadedPt.name_ == "pt");
    REQUIRE(loadedPt.minZoom_ == 2);
    REQUIRE(loadedPt.color_ == QColor(Qt::green));
    REQUIRE(loadedPt.opacity_ == Catch::Approx(0.5));
    REQUIRE(loadedPt.image_ == "img");
    REQUIRE(loadedPt.width_ == Catch::Approx(7.0));
    Label loadedLb = loaded.label(0);
    REQUIRE(loadedLb.visible_);
    REQUIRE(loadedLb.text_ == "[name]");
    REQUIRE(loadedLb.color_ == QColor(Qt::yellow));
    REQUIRE(loadedLb.height_ == Catch::Approx(8.0));

    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("StyleLayer removeSubLayer for points", "[StyleLayer]")
{
    StyleLayer layer("ds", "k", ST_POINT);
    Point p1;
    p1.width_ = 1.0;
    Point p2;
    p2.width_ = 2.0;
    layer.setSubLayerPoint(0, p1);
    layer.setSubLayerPoint(1, p2);
    REQUIRE(layer.subLayerCount() == 2);
    layer.removeSubLayer(0);
    REQUIRE(layer.subLayerCount() == 1);
    REQUIRE(layer.subLayerPoint(0).width_ == Catch::Approx(2.0));
}
