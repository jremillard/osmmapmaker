
#include "osmdatadownload.h"

// Allow any format of input files (XML, PBF, ...)

OsmDataDownload::OsmDataDownload(QDomNode projectNode)
{
	userName_ = "OSM";
	dataName_ = "OSM";
	//importFile("C:\\Remillard\\Documents\\osmmapmaker\\groton.osm");
}

OsmDataDownload::~OsmDataDownload()
{
}

void OsmDataDownload::import()
{
}


