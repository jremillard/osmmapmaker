#pragma once

#include "project.h"

#include <QImage>

#include <mapnik/map.hpp>

class Render
{
public:
	Render(Project *project);

	QImage RenderImage();

	void SetupZoomAtCenter(int imageWithPixels, int imageHeightPixels, double centerX, double centerY, double pixelResolution);
	void SetupZoomBoundingBox(int imageWithPixels, int imageHeightPixels, double left, double right, double bottom, double top);


private:
	mapnik::Map map_;
};
