#pragma once

#include "osmdata.h"

#include <QDomNode>

class OsmDataFile : public OsmData
{
public:
	OsmDataFile(QDomElement projectNode);
	~OsmDataFile();

	void importData(SQLite::Database &db);

private:
	QString fileName_;

};







