#pragma once

#include "project.h"

#include <QImage>

#include <mapnik/map.hpp>

class Render
{
public:
	Render(Project *project, double leftLinear, double bottomLinear, double pixelResolution, int imageWithPixels, int imageHeightPixels);

	QImage RenderImage();

private:
	mapnik::Map map_;
};
