#pragma once

#include <QString>
#include <QtXml>

struct BoundingBox {
    double left_;
    double right_;
    double top_;
    double bottom_;
};

/// Abstract description of an export target for rendered data.
class Output {
public:
    Output(QString name);
    Output(QDomElement ouputNode);

    virtual ~Output();

    QString name();
    void setName(QString name);

    virtual void saveXML(QDomDocument& doc, QDomElement& layerElement);

protected:
    QString name_;
};

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
