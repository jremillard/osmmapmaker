#pragma once

#include "osmdata.h"

#include <QDomNode>

class OsmDataFile : public OsmData
{
public:
	OsmDataFile();
	OsmDataFile(QDomElement projectNode);

	~OsmDataFile();

	void importData(SQLite::Database &db);

	QString localFile();
	void    SetLocalFile(QString file);

private:
	QString fileName_;

};







