#pragma once

#include <QPainter>
#include <QPointF>
#include <QString>
#include <vector>

/// Static helper for drawing text along polylines.
class TextOnPath {
public:
    /// Computes the total length of a polyline in pixels.
    static double polylineLength(const std::vector<QPointF>& pts);

    /// Returns the point and tangent angle at a given distance along the
    /// polyline. Distances outside the line are clamped to the start or end.
    static void pointAlongPolyline(const std::vector<QPointF>& pts,
        double distance,
        QPointF* pos,
        double* angle);

    /// Draws text centered along a polyline by placing each glyph individually.
    static void drawTextOnPath(QPainter& painter,
        const QString& text,
        const QFont& font,
        const std::vector<QPointF>& pts,
        const QColor& color,
        double offset);

    /// Legacy algorithm that repeats the entire text string along the line at
    /// regular intervals.
    static void drawTextOnPathLegacy(QPainter& painter,
        const QString& text,
        const QFont& font,
        const std::vector<QPointF>& pts,
        const QColor& color,
        double offset,
        double maxGap);
};
