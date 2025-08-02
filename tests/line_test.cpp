#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "stylelayer.h"
#include <QtXml>
#include <QCoreApplication>
#include <QEvent>
using Catch::Approx;

TEST_CASE("Line default constructor", "[Line]")
{
    Line line;
    REQUIRE(line.visible_ == true);
    REQUIRE(line.color_ == QColor(Qt::black));
    REQUIRE(line.minZoom_ == 0);
    REQUIRE(line.opacity_ == 1);
    REQUIRE(line.casingColor_ == QColor(Qt::white));
    REQUIRE(line.width_ == Approx(1.0));
    REQUIRE(line.casingWidth_ == Approx(0.0));
    REQUIRE(line.smooth_ == Approx(0.0));
    REQUIRE(line.dashArray_.empty());
}

TEST_CASE("StyleLayer parses line with dash array", "[Line]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    QDomDocument doc;
    QString xml = "<layer dataSource='ds' k='k' type='line'>"
                  " <subLayer name='sl' visible='true' minZoom='5'>"
                  "  <line>"
                  "   <color>#ff0000</color>"
                  "   <casingColor>#00ff00</casingColor>"
                  "   <width>2</width>"
                  "   <casingWidth>1</casingWidth>"
                  "   <opacity>0.5</opacity>"
                  "   <smooth>3</smooth>"
                  "   <dashArray>4,1, 2,2</dashArray>"
                  "  </line>"
                  " </subLayer>"
                  "</layer>";
    doc.setContent(xml);
    StyleLayer layer(doc.documentElement());
    REQUIRE(layer.layerType() == ST_LINE);
    Line parsed = layer.subLayerLine(0);
    REQUIRE(parsed.name_ == "sl");
    REQUIRE(parsed.minZoom_ == 5);
    REQUIRE(parsed.color_ == QColor("#ff0000"));
    REQUIRE(parsed.casingColor_ == QColor("#00ff00"));
    REQUIRE(parsed.width_ == Approx(2.0));
    REQUIRE(parsed.casingWidth_ == Approx(1.0));
    REQUIRE(parsed.opacity_ == Approx(0.5));
    REQUIRE(parsed.smooth_ == Approx(3.0));
    REQUIRE(parsed.dashArray_.size() == 2);
    REQUIRE(parsed.dashArray_[0].first == Approx(4.0));
    REQUIRE(parsed.dashArray_[0].second == Approx(1.0));
    REQUIRE(parsed.dashArray_[1].first == Approx(2.0));
    REQUIRE(parsed.dashArray_[1].second == Approx(2.0));
}

TEST_CASE("StyleLayer saves line with dash array", "[Line]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    StyleLayer layer("ds", "k", ST_LINE);
    Line line;
    line.name_ = "sl";
    line.minZoom_ = 4;
    line.color_ = QColor("#111111");
    line.casingColor_ = QColor("#222222");
    line.width_ = 1.5;
    line.casingWidth_ = 0.5;
    line.opacity_ = 0.7;
    line.smooth_ = 2;
    line.dashArray_.push_back({ 1, 2 });
    line.dashArray_.push_back({ 3, 4 });
    layer.setSubLayerLine(0, line);

    QDomDocument doc;
    QDomElement root = doc.createElement("layer");
    layer.saveXML(doc, root);

    QString dashStr = root.firstChildElement("subLayer")
                          .firstChildElement("line")
                          .firstChildElement("dashArray")
                          .text();
    REQUIRE(dashStr == "1,2,3,4");
}

TEST_CASE("StyleLayer parses line without dash array", "[Line]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    QDomDocument doc;
    QString xml = "<layer dataSource='ds' k='k' type='line'>"
                  " <subLayer name='' visible='false' minZoom='1'>"
                  "  <line>"
                  "   <color>#010101</color>"
                  "   <casingColor>#020202</casingColor>"
                  "   <width>3</width>"
                  "   <casingWidth>0</casingWidth>"
                  "   <opacity>1</opacity>"
                  "   <smooth>0</smooth>"
                  "   <dashArray></dashArray>"
                  "  </line>"
                  " </subLayer>"
                  "</layer>";
    doc.setContent(xml);
    StyleLayer layer(doc.documentElement());
    Line parsed = layer.subLayerLine(0);
    REQUIRE_FALSE(parsed.visible_);
    REQUIRE(parsed.minZoom_ == 1);
    REQUIRE(parsed.color_ == QColor("#010101"));
    REQUIRE(parsed.casingColor_ == QColor("#020202"));
    REQUIRE(parsed.width_ == Approx(3.0));
    REQUIRE(parsed.casingWidth_ == Approx(0.0));
    REQUIRE(parsed.opacity_ == Approx(1.0));
    REQUIRE(parsed.smooth_ == Approx(0.0));
    REQUIRE(parsed.dashArray_.empty());
}
