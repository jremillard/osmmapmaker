#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "stylelayer.h"
#include <QCoreApplication>

TEST_CASE("Label default values", "[Label]")
{
    int argc = 0;
    QCoreApplication app(argc, nullptr);
    Label lb;
    REQUIRE(lb.text_ == "[name]");
    REQUIRE(lb.height_ == Catch::Approx(10.0));
    REQUIRE(lb.haloSize_ == Catch::Approx(0.0));
    REQUIRE(lb.haloColor_ == QColor(Qt::black));
    REQUIRE(lb.maxWrapWidth_ == Catch::Approx(30.0));
    REQUIRE(lb.offsetY_ == Catch::Approx(0.0));
    REQUIRE(lb.fontWeight == 400);
    REQUIRE(lb.priority_ == 0);
    app.processEvents();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}

TEST_CASE("Label mapnikText returns text", "[Label]")
{
    Label lb;
    lb.text_ = "Custom";
    REQUIRE(lb.mapnikText() == "Custom");
}
