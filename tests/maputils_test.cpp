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
