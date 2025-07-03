
#include "osmdatadirectdownload.h"

// Allow any format of input files (XML, PBF, ...)

OsmDataDirectDownload::OsmDataDirectDownload(QDomNode projectNode)
{
    userName_ = "OSM";
    dataName_ = "OSM";
    // importFile("C:\\Remillard\\Documents\\osmmapmaker\\groton.osm");
}

OsmDataDirectDownload::~OsmDataDirectDownload()
{
}

void OsmDataDirectDownload::importData(SQLite::Database& db)
{
}

void OsmDataDirectDownload::saveXML(QDomDocument& doc, QDomElement& toElement)
{
    toElement = doc.createElement("openStreetMapDirectDownload");

    DataSource::saveXML(doc, toElement);
}
