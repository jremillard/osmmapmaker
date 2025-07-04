#pragma once

#include <QObject>
#include <QPixmap>

/// Simple cache for rendered map tiles.
class TileCache : public QObject {
    Q_OBJECT

public:
    TileCache();
    ~TileCache();

    bool GetTile(double zoom, int tileRow, int tileCol, uint renderVersion, QPixmap* image);

signals:
    void newImageAvailable();

private:
};
