
#include "batchtileoutput.h"
#include <cmath>
#include <QtXml>

void BatchTileOutput::generateTiles(Project* project, TileOutput& output)
{
    generateTiles(project, output, {});
}

void BatchTileOutput::generateTiles(Project* project, TileOutput& output,
    const std::function<bool(int, int)>& progress)
{
    using std::filesystem::path;

    int total = 0;
    if (progress) {
        total = processTiles(project, output, true, {}, 0);
        progress(0, total);
    }

    processTiles(project, output, false, progress, total);
}

int BatchTileOutput::processTiles(Project* project, TileOutput& output, bool countOnly,
    const std::function<bool(int, int)>& progress, int total)
{
    using std::filesystem::path;

    path tileDir;

    if (output.outputDirectory().size() == 0) {
        QString outputDirectory = QString::fromStdString(project->projectPath().string());
        outputDirectory.replace(".xml", "", Qt::CaseInsensitive);

        tileDir = path(outputDirectory.toStdString());
        tileDir /= output.name().toStdString();
    } else {
        tileDir = path(output.outputDirectory().toStdString());
        tileDir /= output.name().toStdString();
    }

    if (!countOnly) {
        for (int z = 0; z <= 25; ++z) {
            path zoomDir = tileDir / QString::number(z).toStdString();
            std::error_code ec;
            remove_all(zoomDir, ec);
        }
    }

    int tileSize = output.tileSizePixels();

    double latitude = 42.608089;
    double lon = -71.571152;
    double distDeg = 0.1;

    auto ll0 = std::pair<double, double>(lon - distDeg, latitude + distDeg);
    auto ll1 = std::pair<double, double>(lon + distDeg, latitude - distDeg);

    RenderQT render1x(project, 1);
    RenderQT render2x(project, 2);

    int done = 0;
    int count = 0;

    for (int z = output.minZoom(); z <= output.maxZoom(); ++z) {
        std::string zoomStr = QString::number(z).toStdString();
        path zoomDir = tileDir / zoomStr;

        if (!countOnly)
            create_directories(zoomDir);

        std::pair<double, double> px0 = fromLLtoPixel(tileSize, ll0, z);
        std::pair<double, double> px1 = fromLLtoPixel(tileSize, ll1, z);

        for (int x = int(px0.first / tileSize); x < int(px1.first / tileSize) + 1; ++x) {
            if ((x < 0) || (x >= std::pow(2, z)))
                continue;

            std::string str_x = QString::number(x).toStdString();

            if (!countOnly) {
                if (!exists(tileDir / zoomStr / str_x))
                    create_directories(tileDir / zoomStr / str_x);
            }

            int yEnd = int(px1.second / tileSize) + 1;

            for (int y = int(px0.second / tileSize); y < yEnd; ++y) {
                if ((y < 0) || (y >= std::pow(2, z)))
                    continue;

                std::string str_y = QString::number(y).toStdString();

                if (output.resolution1x()) {
                    count++;
                    if (!countOnly) {
                        path tile_uri1x = tileDir / zoomStr / str_x / (str_y + ".png");
                        renderTile(project, render1x, tile_uri1x, tileSize, 1, x, y, z);
                        done++;
                        if (progress && !progress(done, total))
                            return count;
                    }
                }

                if (output.resolution2x()) {
                    count++;
                    if (!countOnly) {
                        path tile_uri2x = tileDir / zoomStr / str_x / (str_y + "@2x.png");
                        renderTile(project, render2x, tile_uri2x, tileSize, 2, x, y, z);
                        done++;
                        if (progress && !progress(done, total))
                            return count;
                    }
                }
            }
        }
    }

    return count;
}

void BatchTileOutput::renderTile(Project* project, RenderQT& render, const std::filesystem::path& imagePath, int tileSize, int resolutionScale, int x, int y, int z)
{
    std::pair<int, int> p0(x * tileSize, (y + 1) * tileSize);
    std::pair<int, int> p1((x + 1) * tileSize, y * tileSize);

    std::pair<double, double> l0 = fromPixelToLL(tileSize, p0, z);
    std::pair<double, double> l1 = fromPixelToLL(tileSize, p1, z);

    double x0, y0;
    project->convertDataToMap(l0.first, l0.second, &x0, &y0);

    double x1, y1;
    project->convertDataToMap(l1.first, l1.second, &x1, &y1);

    render.SetupZoomBoundingBox(tileSize * resolutionScale, tileSize * resolutionScale, x0, x1, y0, y1);

    QImage img = render.RenderImage();
    img.save(QString::fromStdString(imagePath.string()));
}

std::pair<double, double> BatchTileOutput::fromPixelToLL(int tileSize, std::pair<double, double> px, int zoom)
{
    std::vector<double> Bc;
    std::vector<double> Cc;
    std::vector<double> zc;
    std::vector<double> Ac;

    const double PI = std::atan(1.0) * 4;

    int c = 256;
    for (int z = 0; z <= 20; ++z) {
        double e = c / 2;

        Bc.push_back(c / 360.0);
        Cc.push_back(c / (2 * PI));
        zc.push_back(e);
        Ac.push_back(c);
        c *= 2;
    }

    const double RAD_TO_DEG = 180.0 / PI;

    double e = zc[zoom];
    double f = (px.first - e) / Bc[zoom];
    double g = (px.second - e) / -Cc[zoom];
    double h = RAD_TO_DEG * (2 * atan(exp(g)) - 0.5 * PI);
    return std::pair<double, double>(f, h);
}

std::pair<double, double> BatchTileOutput::fromLLtoPixel(int tileSize, std::pair<double, double> ll, int zoom)
{
    std::vector<double> Bc;
    std::vector<double> Cc;
    std::vector<double> zc;
    std::vector<double> Ac;

    const double PI = std::atan(1.0) * 4;

    int c = 256;
    for (int z = 0; z <= 20; ++z) {
        double e = c / 2;

        Bc.push_back(c / 360.0);
        Cc.push_back(c / (2 * PI));
        zc.push_back(e);
        Ac.push_back(c);
        c *= 2;
    }

    const double DEG_TO_RAD = PI / 180.0;

    double d = zc[zoom];
    double e = round(d + ll.first * Bc[zoom]);
    double f = std::min<double>(std::max<double>(sin(DEG_TO_RAD * ll.second), -0.9999), 0.9999);
    double g = round(d + 0.5 * log((1 + f) / (1 - f)) * -Cc[zoom]);
    return std::pair<double, double>(e, g);
}
