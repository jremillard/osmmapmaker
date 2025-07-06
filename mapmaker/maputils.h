#pragma once

#include <cmath>
#include <QPointF>

struct TileXY {
    double x;
    double y;
};

/// Utility functions for geographic calculations.
/// Returns the meters per pixel for a given latitude (degrees)
/// and zoom level.
double metersPerPixel(double latDeg, int zoom);

/// Convert longitude/latitude to fractional tile coordinates at a zoom level.
TileXY lonLatToTile(double lonDeg, double latDeg, int zoom);

/// Convert tile coordinates back to longitude/latitude.
void tileToLonLat(const TileXY& t, int zoom, double& lonDeg, double& latDeg);

/// Convert longitude/latitude to pixel coordinates.
QPointF lonLatToPixel(double lonDeg, double latDeg, int zoom, int tileSize = 256);

/// Convert pixel coordinates to longitude/latitude.
void pixelToLonLat(const QPointF& p, int zoom, int tileSize, double& lonDeg, double& latDeg);
