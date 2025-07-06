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
    Output(QDomElement outputNode);

    virtual ~Output();

    QString name();
    void setName(QString name);

    virtual void saveXML(QDomDocument& doc, QDomElement& layerElement);

protected:
    QString name_;
};

