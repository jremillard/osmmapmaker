#include "output.h"
#include <QtXml>

Output::Output(QString nameIn)
    : name_(nameIn)
{
}

Output::Output(QDomElement ouputNode)
{
    name_ = ouputNode.attributes().namedItem("name").nodeValue();
}

Output::~Output()
{
}

QString Output::name()
{
    return name_;
}

void Output::setName(QString name)
{
    name_ = name;
}

void Output::saveXML(QDomDocument& doc, QDomElement& ouputNode)
{
    ouputNode.setAttribute("name", name_);
}

TileOutput::TileOutput(QString name)
    : Output(name)
{
    maxZoom_ = 18;
    minZoom_ = 13;
    tileSizePixels_ = 256;
    resolution1x_ = true;
    resolution2x_ = true;
}

TileOutput::TileOutput(QDomElement outputElement)
    : Output(outputElement)
{
    maxZoom_ = outputElement.firstChildElement("maxZoom").text().toInt();
    minZoom_ = outputElement.firstChildElement("minZoom").text().toInt();
    tileSizePixels_ = outputElement.firstChildElement("tileSize").text().toInt();
    resolution1x_ = outputElement.firstChildElement("resolution1x").text() == "true";
    resolution2x_ = outputElement.firstChildElement("resolution2x").text() == "true";
    outputDirectory_ = outputElement.firstChildElement("directory").text();
    // BoundingBox boundingBox_;
}

TileOutput::~TileOutput()
{
}

void TileOutput::saveXML(QDomDocument& doc, QDomElement& outputElement)
{
    outputElement = doc.createElement("tileOutput");

    Output::saveXML(doc, outputElement);

    QDomElement maxZoomNode = doc.createElement("maxZoom");
    maxZoomNode.appendChild(doc.createTextNode(QString::number(maxZoom_)));
    outputElement.appendChild(maxZoomNode);

    QDomElement minZoomNode = doc.createElement("minZoom");
    minZoomNode.appendChild(doc.createTextNode(QString::number(minZoom_)));
    outputElement.appendChild(minZoomNode);

    QDomElement tileSizeNode = doc.createElement("tileSize");
    tileSizeNode.appendChild(doc.createTextNode(QString::number(tileSizePixels_)));
    outputElement.appendChild(tileSizeNode);

    QDomElement res1xNode = doc.createElement("resolution1x");
    res1xNode.appendChild(doc.createTextNode(resolution1x_ ? "true" : "false"));
    outputElement.appendChild(res1xNode);

    QDomElement res2xNode = doc.createElement("resolution2x");
    res2xNode.appendChild(doc.createTextNode(resolution2x_ ? "true" : "false"));
    outputElement.appendChild(res2xNode);

    QDomElement directoryNode = doc.createElement("directory");
    directoryNode.appendChild(doc.createTextNode(outputDirectory_));
    outputElement.appendChild(directoryNode);

    // BoundingBox boundingBox_;
}

int TileOutput::maxZoom() const
{
    return maxZoom_;
}

void TileOutput::setMaxZoom(int z)
{
    maxZoom_ = z;
}

int TileOutput::minZoom() const
{
    return minZoom_;
}

void TileOutput::setMinZoom(int z)
{
    minZoom_ = z;
}

int TileOutput::tileSizePixels() const
{
    return tileSizePixels_;
}

void TileOutput::setTileSizePixels(int s)
{
    tileSizePixels_ = s;
}

bool TileOutput::resolution1x()
{
    return resolution1x_;
}

void TileOutput::setResolution1x(bool r)
{
    resolution1x_ = r;
}

bool TileOutput::resolution2x()
{
    return resolution2x_;
}

void TileOutput::setResolution2x(bool r)
{
    resolution2x_ = r;
}

QString TileOutput::outputDirectory()
{
    return outputDirectory_;
}

void TileOutput::setOutputDirectory(QString dir)
{
    outputDirectory_ = dir;
}
