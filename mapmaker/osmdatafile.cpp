#include "osmdatafile.h"

OsmDataFile::OsmDataFile(QDomElement projectNode)
    : OsmData(projectNode)
{
    QDomElement dataSourceE = projectNode.firstChildElement("fileName");
    fileName_ = dataSourceE.text();
}

OsmDataFile::OsmDataFile()
{
}

OsmDataFile::~OsmDataFile()
{
}

QString OsmDataFile::localFile()
{
    return fileName_;
}

void OsmDataFile::SetLocalFile(QString file)
{
    fileName_ = file;
}

void OsmDataFile::importData(SQLite::Database& db)
{
    importFile(db, fileName_);
}

void OsmDataFile::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("openStreetMapFileSource");

    DataSource::saveXML(doc, toElement);

    QDomElement fileNameElement = doc.createElement("fileName");
    fileNameElement.appendChild(doc.createTextNode(fileName_));

    toElement.appendChild(fileNameElement);
}
