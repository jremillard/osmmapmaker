#include "imageoutput.h"
#include <QtXml>

ImageOutput::ImageOutput(QString name)
    : Output(name)
{
    widthPixels_ = 1024;
    heightPixels_ = 1024;
    boundingBox_ = { 0, 0, 0, 0 };
    outputFile_.clear();
}

ImageOutput::ImageOutput(QDomElement outputElement)
    : Output(outputElement)
{
    widthPixels_ = outputElement.firstChildElement("width").text().toInt();
    heightPixels_ = outputElement.firstChildElement("height").text().toInt();
    outputFile_ = outputElement.firstChildElement("file").text();

    QDomElement bbox = outputElement.firstChildElement("boundingBox");
    if (!bbox.isNull()) {
        boundingBox_.left_ = bbox.attribute("left").toDouble();
        boundingBox_.right_ = bbox.attribute("right").toDouble();
        boundingBox_.top_ = bbox.attribute("top").toDouble();
        boundingBox_.bottom_ = bbox.attribute("bottom").toDouble();
    } else {
        boundingBox_ = { 0, 0, 0, 0 };
    }
}

ImageOutput::~ImageOutput()
{
}

void ImageOutput::saveXML(QDomDocument& doc, QDomElement& outputElement)
{
    outputElement = doc.createElement("imageOutput");

    Output::saveXML(doc, outputElement);

    QDomElement widthNode = doc.createElement("width");
    widthNode.appendChild(doc.createTextNode(QString::number(widthPixels_)));
    outputElement.appendChild(widthNode);

    QDomElement heightNode = doc.createElement("height");
    heightNode.appendChild(doc.createTextNode(QString::number(heightPixels_)));
    outputElement.appendChild(heightNode);

    if (!outputFile_.isEmpty()) {
        QDomElement fileNode = doc.createElement("file");
        fileNode.appendChild(doc.createTextNode(outputFile_));
        outputElement.appendChild(fileNode);
    }

    QDomElement bboxNode = doc.createElement("boundingBox");
    bboxNode.setAttribute("left", QString::number(boundingBox_.left_));
    bboxNode.setAttribute("right", QString::number(boundingBox_.right_));
    bboxNode.setAttribute("top", QString::number(boundingBox_.top_));
    bboxNode.setAttribute("bottom", QString::number(boundingBox_.bottom_));
    outputElement.appendChild(bboxNode);
}

int ImageOutput::widthPixels() const
{
    return widthPixels_;
}

void ImageOutput::setWidthPixels(int w)
{
    widthPixels_ = w;
}

int ImageOutput::heightPixels() const
{
    return heightPixels_;
}

void ImageOutput::setHeightPixels(int h)
{
    heightPixels_ = h;
}

BoundingBox ImageOutput::boundingBox() const
{
    return boundingBox_;
}

void ImageOutput::setBoundingBox(const BoundingBox& bb)
{
    boundingBox_ = bb;
}

QString ImageOutput::outputFile() const
{
    return outputFile_;
}

void ImageOutput::setOutputFile(const QString& file)
{
    outputFile_ = file;
}
