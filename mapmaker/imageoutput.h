#pragma once

#include "output.h"
#include <QString>
#include <QtXml>

/// Output that renders a single image within an optional bounding box.
class ImageOutput : public Output {
public:
    ImageOutput(QString name);
    ImageOutput(QDomElement ouputNode);
    ~ImageOutput();

    int widthPixels() const;
    void setWidthPixels(int w);

    int heightPixels() const;
    void setHeightPixels(int h);

    BoundingBox boundingBox() const;
    void setBoundingBox(const BoundingBox& bb);

    virtual void saveXML(QDomDocument& doc, QDomElement& layerElement);

private:
    int widthPixels_;
    int heightPixels_;
    BoundingBox boundingBox_;
};
