#include "textonpath.h"
#include <QPainterPath>
#include <QPainterPathStroker>
#include <algorithm>
#include <cmath>

// Compute the total length of a polyline.
double TextOnPath::polylineLength(const std::vector<QPointF>& pts)
{
    double len = 0.0;
    for (size_t i = 1; i < pts.size(); ++i) {
        double dx = pts[i].x() - pts[i - 1].x();
        double dy = pts[i].y() - pts[i - 1].y();
        len += std::hypot(dx, dy);
    }
    return len;
}

// Return the point and tangent angle at a given distance along the polyline.
void TextOnPath::pointAlongPolyline(const std::vector<QPointF>& pts,
    double distance,
    QPointF* pos,
    double* angle)
{
    if (pts.size() < 2) {
        if (pos)
            *pos = pts.empty() ? QPointF() : pts.front();
        if (angle)
            *angle = 0.0;
        return;
    }

    double pathLen = polylineLength(pts);
    double d = std::clamp(distance, 0.0, pathLen);
    for (size_t i = 1; i < pts.size(); ++i) {
        double segLen = std::hypot(pts[i].x() - pts[i - 1].x(),
            pts[i].y() - pts[i - 1].y());
        if (d <= segLen || i + 1 == pts.size()) {
            double ratio = segLen > 0 ? std::min(d, segLen) / segLen : 0.0;
            double x = pts[i - 1].x() + ratio * (pts[i].x() - pts[i - 1].x());
            double y = pts[i - 1].y() + ratio * (pts[i].y() - pts[i - 1].y());
            if (pos)
                *pos = QPointF(x, y);
            if (angle)
                *angle = std::atan2(pts[i].y() - pts[i - 1].y(),
                    pts[i].x() - pts[i - 1].x());
            return;
        }
        d -= segLen;
    }

    if (pos)
        *pos = pts.back();
    if (angle)
        *angle = std::atan2(pts.back().y() - pts[pts.size() - 2].y(),
            pts.back().x() - pts[pts.size() - 2].x());
}

// Draw text along a polyline by positioning each glyph individually.
void TextOnPath::drawTextOnPath(QPainter& painter,
    const QString& text,
    const QFont& font,
    const std::vector<QPointF>& pts,
    const QColor& color,
    double offset)
{
    if (pts.size() < 2 || text.isEmpty())
        return;

    QFontMetricsF metrics(font);
    double pathLen = polylineLength(pts);
    double textWidth = metrics.horizontalAdvance(text);
    if (textWidth > pathLen)
        return; // not enough space

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    double pos = (pathLen - textWidth) / 2.0;
    for (int i = 0; i < text.size(); ++i) {
        QString ch = text.mid(i, 1);
        double cw = metrics.horizontalAdvance(ch);
        QPointF pt;
        double angle;
        pointAlongPolyline(pts, pos + cw / 2.0, &pt, &angle);
        QTransform t;
        t.translate(pt.x(), pt.y());
        double angleDeg = -angle * 180.0 / M_PI;
        if (angleDeg > 90.0)
            angleDeg -= 180.0;
        else if (angleDeg < -90.0)
            angleDeg += 180.0;
        t.rotate(angleDeg);
        QPainterPath glyph;
        glyph.addText(-cw / 2.0, offset, font, ch);
        painter.drawPath(t.map(glyph));
        pos += cw;
    }

    painter.restore();
}

// Legacy algorithm that repeats the entire text string along the polyline.
void TextOnPath::drawTextOnPathLegacy(QPainter& painter,
    const QString& text,
    const QFont& font,
    const std::vector<QPointF>& pts,
    const QColor& color,
    double offset,
    double maxGap)
{
    if (pts.size() < 2 || text.isEmpty())
        return;

    QFontMetricsF metrics(font);
    double pathLen = polylineLength(pts);
    double textWidth = metrics.horizontalAdvance(text);

    // Gap parameter controls minimum separation between repeated labels.
    if (maxGap <= 0)
        maxGap = 400.0;

    // Calculate pitch between labels.
    double maxPitch = textWidth + maxGap;
    double gapCount = std::ceil(pathLen / maxPitch);
    if (gapCount <= 1)
        gapCount = 2.0;

    double pitch = pathLen / gapCount;
    if (pitch < textWidth / 2.2)
        return; // too small, skip drawing

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    double lengthTotal = 0.0;
    double nextLabel = pitch;
    for (size_t i = 1; i < pts.size(); ++i) {
        QPointF p1 = pts[i - 1];
        QPointF p2 = pts[i];
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double segLen = std::hypot(dx, dy);

        while (lengthTotal + segLen > nextLabel) {
            double used = nextLabel - lengthTotal;
            double ratio = segLen > 0.0 ? used / segLen : 0.0;
            segLen -= used;
            lengthTotal += used;
            nextLabel += pitch;

            double cx = p1.x() + ratio * dx;
            double cy = p1.y() + ratio * dy;
            double angle = std::atan2(dy, dx);

            QTransform t;
            t.translate(cx, cy);
            double angleDeg = -angle * 180.0 / M_PI;
            if (angleDeg > 90)
                angleDeg -= 180;
            else if (angleDeg < -90)
                angleDeg += 180;
            t.rotate(angleDeg);

            QPainterPath path;
            path.addText(-textWidth / 2.0, offset, font, text);

            painter.drawPath(t.map(path));
        }

        lengthTotal += segLen;
    }

    painter.restore();
}
