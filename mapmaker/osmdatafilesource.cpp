#include "osmdatafilesource.h"

OsmDataFileSource::OsmDataFileSource(QDomElement projectNode)
    : DataSource(projectNode)
{
    QDomElement dataSourceE = projectNode.firstChildElement("fileName");
    fileName_ = dataSourceE.text();
}

OsmDataFileSource::OsmDataFileSource()
{
}

OsmDataFileSource::~OsmDataFileSource()
{
}

QString OsmDataFileSource::localFile()
{
    return fileName_;
}

void OsmDataFileSource::SetLocalFile(QString file)
{
    fileName_ = file;
}

void OsmDataFileSource::importData(RenderDatabase& db)
{
    OsmiumImporter::importFile(db, fileName_, dataName());
}

void OsmDataFileSource::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("openStreetMapFileSource");

    DataSource::saveXML(doc, toElement);

    QDomElement fileNameElement = doc.createElement("fileName");
    fileNameElement.appendChild(doc.createTextNode(fileName_));

    toElement.appendChild(fileNameElement);
}
