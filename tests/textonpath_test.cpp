#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <cmath>
#include "textonpath.h"

TEST_CASE("polylineLength computes correct distance", "[textonpath]")
{
    std::vector<QPointF> pts { QPointF(0, 0), QPointF(3, 4), QPointF(3, 8) };
    REQUIRE(TextOnPath::polylineLength(pts) == Catch::Approx(9.0));
}

TEST_CASE("pointAlongPolyline returns position and angle", "[textonpath]")
{
    std::vector<QPointF> pts { QPointF(0, 0), QPointF(3, 0), QPointF(3, 4) };
    QPointF pos;
    double angle = 0.0;
    TextOnPath::pointAlongPolyline(pts, 4.0, &pos, &angle);
    REQUIRE(pos.x() == Catch::Approx(3.0));
    REQUIRE(pos.y() == Catch::Approx(1.0));
    REQUIRE(angle == Catch::Approx(M_PI_2));
}

TEST_CASE("pointAlongPolyline clamps distance", "[textonpath]")
{
    std::vector<QPointF> pts { QPointF(0, 0), QPointF(3, 0), QPointF(3, 4) };
    QPointF pos;
    double angle = 0.0;

    TextOnPath::pointAlongPolyline(pts, -5.0, &pos, &angle);
    REQUIRE(pos.x() == Catch::Approx(0.0));
    REQUIRE(pos.y() == Catch::Approx(0.0));
    REQUIRE(angle == Catch::Approx(0.0));

    TextOnPath::pointAlongPolyline(pts, 10.0, &pos, &angle);
    REQUIRE(pos.x() == Catch::Approx(3.0));
    REQUIRE(pos.y() == Catch::Approx(4.0));
    REQUIRE(angle == Catch::Approx(M_PI_2));
}
