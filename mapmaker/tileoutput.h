#pragma once

#include "output.h"
#include <QString>
#include <QtXml>

/// Output that renders map tiles into a directory structure.
class TileOutput : public Output {
public:
    TileOutput(QString name);
    TileOutput(QDomElement ouputNode);
    ~TileOutput();

    int maxZoom() const;
    void setMaxZoom(int z);

    int minZoom() const;
    void setMinZoom(int z);

    int tileSizePixels() const;
    void setTileSizePixels(int s);

    bool resolution1x();
    void setResolution1x(bool r);

    bool resolution2x();
    void setResolution2x(bool r);

    QString outputDirectory();
    void setOutputDirectory(QString dir);

    virtual void saveXML(QDomDocument& doc, QDomElement& layerElement);

private:
    int maxZoom_;
    int minZoom_;
    int tileSizePixels_;
    bool resolution1x_;
    bool resolution2x_;
    QString outputDirectory_;
    BoundingBox boundingBox_;
};
