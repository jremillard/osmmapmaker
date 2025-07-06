#include "output.h"
#include <QtXml>

Output::Output(QString nameIn)
    : name_(nameIn)
{
}

Output::Output(QDomElement outputNode)
{
    name_ = outputNode.attributes().namedItem("name").nodeValue();
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

void Output::saveXML(QDomDocument& doc, QDomElement& outputNode)
{
    outputNode.setAttribute("name", name_);
}
