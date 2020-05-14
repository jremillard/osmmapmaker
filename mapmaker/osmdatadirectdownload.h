#pragma once

#include "osmdata.h"

#include <QDomNode>

class OsmDataDirectDownload : public OsmData
{
public:
	OsmDataDirectDownload(QDomNode projectNode);
	~OsmDataDirectDownload();

	void importData(SQLite::Database &db);

	virtual void saveXML(QDomDocument &doc, QDomElement &toElement);

};





