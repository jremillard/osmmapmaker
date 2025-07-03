#include <catch2/catch_test_macros.hpp>
#include "stylelayer.h"
#include <QtXml>

TEST_CASE("Label default priority", "[StyleLayer]") {
    Label lb;
    REQUIRE(lb.priority_ == 0);
}

TEST_CASE("StyleLayer preserves label priority in XML", "[StyleLayer]") {
    StyleLayer layer("ds", "k", ST_POINT);
    Point pt;
    layer.setSubLayerPoint(0, pt);
    Label lb;
    lb.priority_ = 5;
    lb.visible_ = true;
    layer.setLabel(0, lb);

    QDomDocument doc;
    QDomElement elem = doc.createElement("layer");
    layer.saveXML(doc, elem);
    doc.appendChild(elem);

    StyleLayer loaded(elem);
    REQUIRE(loaded.label(0).priority_ == 5);
}
