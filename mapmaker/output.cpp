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
