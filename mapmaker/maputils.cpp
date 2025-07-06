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

TileXY lonLatToTile(double lonDeg, double latDeg, int zoom)
{
    double latRad = latDeg * kDegToRad;
    double n = std::pow(2.0, zoom);
    double x = (lonDeg + 180.0) / 360.0 * n;
    double y = (1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * n;
    return { x, y };
}

void tileToLonLat(const TileXY& t, int zoom, double& lonDeg, double& latDeg)
{
    double n = std::pow(2.0, zoom);
    lonDeg = t.x / n * 360.0 - 180.0;
    double latRad = std::atan(std::sinh(M_PI * (1 - 2 * t.y / n)));
    latDeg = latRad / kDegToRad;
}

QPointF lonLatToPixel(double lonDeg, double latDeg, int zoom, int tileSize)
{
    TileXY t = lonLatToTile(lonDeg, latDeg, zoom);
    return QPointF(t.x * tileSize, t.y * tileSize);
}

void pixelToLonLat(const QPointF& p, int zoom, int tileSize, double& lonDeg, double& latDeg)
{
    TileXY t { p.x() / tileSize, p.y() / tileSize };
    tileToLonLat(t, zoom, lonDeg, latDeg);
}
