#include <catch2/catch_test_macros.hpp>
#include "imageoutput.h"
#include <QtXml>

TEST_CASE("ImageOutput default values", "[ImageOutput]")
{
    ImageOutput out("img");
    REQUIRE(out.widthPixels() == 1024);
    REQUIRE(out.heightPixels() == 1024);
    BoundingBox bb = out.boundingBox();
    REQUIRE(bb.left_ == 0);
    REQUIRE(bb.right_ == 0);
    REQUIRE(bb.top_ == 0);
    REQUIRE(bb.bottom_ == 0);
}

TEST_CASE("ImageOutput setters and getters", "[ImageOutput]")
{
    ImageOutput out("temp");
    out.setWidthPixels(640);
    out.setHeightPixels(480);
    out.setOutputFile("file.png");
    BoundingBox bb { 1, 2, 3, 4 };
    out.setBoundingBox(bb);

    REQUIRE(out.widthPixels() == 640);
    REQUIRE(out.heightPixels() == 480);
    BoundingBox obb = out.boundingBox();
    REQUIRE(obb.left_ == 1);
    REQUIRE(obb.right_ == 2);
    REQUIRE(obb.top_ == 3);
    REQUIRE(obb.bottom_ == 4);
    REQUIRE(out.outputFile() == "file.png");
}

TEST_CASE("ImageOutput saveXML populates document", "[ImageOutput]")
{
    ImageOutput out("xml");
    out.setWidthPixels(800);
    out.setHeightPixels(600);
    BoundingBox bb { -1, 1, 2, -2 };
    out.setBoundingBox(bb);
    out.setOutputFile("out.png");

    QDomDocument doc;
    QDomElement elem;
    out.saveXML(doc, elem);

    REQUIRE(elem.tagName() == "imageOutput");
    REQUIRE(elem.attribute("name") == "xml");
    REQUIRE(elem.firstChildElement("width").text() == "800");
    REQUIRE(elem.firstChildElement("height").text() == "600");
    REQUIRE(elem.firstChildElement("file").text() == "out.png");
    QDomElement b = elem.firstChildElement("boundingBox");
    REQUIRE(b.attribute("left") == "-1");
    REQUIRE(b.attribute("right") == "1");
    REQUIRE(b.attribute("top") == "2");
    REQUIRE(b.attribute("bottom") == "-2");
}

TEST_CASE("ImageOutput constructed from DOM", "[ImageOutput]")
{
    QDomDocument doc;
    QDomElement elem = doc.createElement("imageOutput");
    elem.setAttribute("name", "dom");

    auto addChild = [&](const QString& n, const QString& t) {
        QDomElement child = doc.createElement(n);
        child.appendChild(doc.createTextNode(t));
        elem.appendChild(child);
    };
    addChild("width", "320");
    addChild("height", "240");
    addChild("file", "abc.png");
    QDomElement bb = doc.createElement("boundingBox");
    bb.setAttribute("left", "5");
    bb.setAttribute("right", "6");
    bb.setAttribute("top", "7");
    bb.setAttribute("bottom", "8");
    elem.appendChild(bb);

    ImageOutput loaded(elem);
    REQUIRE(loaded.name() == "dom");
    REQUIRE(loaded.widthPixels() == 320);
    REQUIRE(loaded.heightPixels() == 240);
    REQUIRE(loaded.outputFile() == "abc.png");
    BoundingBox lb = loaded.boundingBox();
    REQUIRE(lb.left_ == 5);
    REQUIRE(lb.right_ == 6);
    REQUIRE(lb.top_ == 7);
    REQUIRE(lb.bottom_ == 8);
}
