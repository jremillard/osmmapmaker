#pragma once

#include "project.h"

#include <QImage>

#include <mapnik/map.hpp>

class Render
{
public:
	Render(Project *project);

	QImage RenderImage(int imageWithPixels, int imageHeightPixels, double leftLinear, double bottomLinear, double pixelResolution);

private:
	mapnik::Map map_;
};
