
#include "osmdataextractdownload.h"

// Allow any format of input files (XML, PBF, ...)

OsmDataExtractDownload::OsmDataExtractDownload(QDomNode projectNode)
{
	userName_ = "OSM";
	dataName_ = "OSM";
	//importFile("C:\\Remillard\\Documents\\osmmapmaker\\groton.osm");
}

OsmDataExtractDownload::~OsmDataExtractDownload()
{
}

void OsmDataExtractDownload::importData(SQLite::Database &db)
{
}


