#pragma once

#include "osmdata.h"

class OsmDataExtractDownload : public OsmData
{
public:
	OsmDataExtractDownload(QDomNode projectNode);
	~OsmDataExtractDownload();

	void importData(SQLite::Database &db);

	virtual void saveXML(QDomDocument &doc, QDomElement &toElement);

};





