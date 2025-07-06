#pragma once

#include "project.h"
#include "tileoutput.h"
#include "renderqt.h"
#include <filesystem>
#include <functional>

class BatchTileOutput {
public:
    static void generateTiles(Project* project, TileOutput& output);
    static void generateTiles(Project* project, TileOutput& output,
        const std::function<bool(int, int)>& progress);

    static std::pair<double, double> fromPixelToLL(int tileSize, std::pair<double, double> px, int zoom);
    static std::pair<double, double> fromLLtoPixel(int tileSize, std::pair<double, double> ll, int zoom);
    static void renderTile(Project* project, RenderQT& render, const std::filesystem::path& imagePath, int tileSize, int resolutionScale, int x, int y, int z);

private:
    static int processTiles(Project* project, TileOutput& output, bool countOnly,
        const std::function<bool(int, int)>& progress, int total);
};
