#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "maputils.h"
using Catch::Approx;

TEST_CASE("metersPerPixel at equator zoom0", "[maputils]")
{
    double mpp = metersPerPixel(0.0, 0);
    REQUIRE(mpp == Approx(156543.03392).epsilon(0.00001));
}

TEST_CASE("metersPerPixel varies with latitude and zoom", "[maputils]")
{
    double lat = 45.0; // mid latitude
    int zoom = 3;
    double expected = 156543.03392 * std::cos(lat * M_PI / 180.0) / std::pow(2.0, zoom);
    REQUIRE(metersPerPixel(lat, zoom) == Approx(expected).epsilon(0.00001));
}

TEST_CASE("lonLat to tile and back", "[maputils]")
{
    TileXY t = lonLatToTile(0.0, 0.0, 1);
    REQUIRE(t.x == Approx(1.0));
    REQUIRE(t.y == Approx(1.0));
    double lon, lat;
    tileToLonLat(t, 1, lon, lat);
    REQUIRE(lon == Approx(0.0).margin(0.0001));
    REQUIRE(lat == Approx(0.0).margin(0.0001));
}
