#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "batchtileoutput.h"

TEST_CASE("LL to pixel conversions round trip", "[BatchTileOutput]")
{
    int tileSize = 256;
    double lon = -71.0;
    double lat = 42.0;
    int zoom = 5;

    auto px = BatchTileOutput::fromLLtoPixel(tileSize, { lon, lat }, zoom);
    auto ll = BatchTileOutput::fromPixelToLL(tileSize, px, zoom);

    REQUIRE(ll.first == Catch::Approx(lon).margin(0.1));
    REQUIRE(ll.second == Catch::Approx(lat).margin(0.1));
}
