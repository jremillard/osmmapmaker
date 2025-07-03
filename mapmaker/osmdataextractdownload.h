#pragma once

#include "osmdata.h"

/// Represents an OSM extract that can be downloaded from a mirror
/// and imported using the generic OsmData facilities.
class OsmDataExtractDownload : public OsmData
{
public:
	OsmDataExtractDownload(QDomNode projectNode);
	~OsmDataExtractDownload();

	void importData(SQLite::Database &db);

	virtual void saveXML(QDomDocument &doc, QDomElement &toElement);

};





