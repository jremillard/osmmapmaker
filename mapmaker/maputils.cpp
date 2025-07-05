#include "maputils.h"

namespace {
constexpr double kEarthEquatorialRadiusMeters = 6378137.0;
constexpr double kTileSize = 256.0;
constexpr double kInitialMetersPerPixel = 2 * M_PI * kEarthEquatorialRadiusMeters / kTileSize;
constexpr double kDegToRad = M_PI / 180.0;
}

double metersPerPixel(double latDeg, int zoom)
{
    return kInitialMetersPerPixel * std::cos(latDeg * kDegToRad) / std::pow(2.0, zoom);
}
