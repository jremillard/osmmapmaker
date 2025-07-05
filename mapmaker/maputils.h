#pragma once

#include <cmath>

/// Utility functions for geographic calculations.
/// Returns the meters per pixel for a given latitude (degrees)
/// and zoom level.
double metersPerPixel(double latDeg, int zoom);
