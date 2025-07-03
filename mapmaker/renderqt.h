#pragma once

#include "project.h"
#include <QImage>

/// Renders a project to a QImage using Qt's painting API.
class RenderQT {
public:
    RenderQT(Project* project, int scale = 1);

    QImage RenderImage();

    void SetupZoomAtCenter(int imageWithPixels, int imageHeightPixels, double centerX, double centerY, double pixelResolution);
    void SetupZoomBoundingBox(int imageWithPixels, int imageHeightPixels, double left, double right, double bottom, double top);

private:
    void RenderGeom(QPainter& painter, std::map<int, double>& zoomScale);
    void RenderLabels(QPainter& painter, std::map<int, double>& zoomScale);

    int imageWithPixels_;
    int imageHeightPixels_;
    double left_;
    double right_;
    double bottom_;
    double top_;
    int scale_;

    Project* project_;
};
