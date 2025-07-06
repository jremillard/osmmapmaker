#pragma once

#include "project.h"
#include "tileoutput.h"
#include "renderqt.h"
#include <filesystem>

class BatchTileOutput {
public:
    static void generateTiles(Project* project, TileOutput& output);

    static std::pair<double, double> fromPixelToLL(int tileSize, std::pair<double, double> px, int zoom);
    static std::pair<double, double> fromLLtoPixel(int tileSize, std::pair<double, double> ll, int zoom);
    static void renderTile(Project* project, RenderQT& render, const std::filesystem::path& imagePath, int tileSize, int resolutionScale, int x, int y, int z);
};
