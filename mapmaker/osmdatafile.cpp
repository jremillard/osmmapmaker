#include "osmdatafile.h"


OsmDataFile::OsmDataFile(QDomElement projectNode) : OsmData( projectNode)
{
	QDomElement dataSourceE = projectNode.firstChildElement("fileName");
	fileName_ = dataSourceE.text();
}

OsmDataFile::~OsmDataFile()
{
}

void OsmDataFile::importData(SQLite::Database &db)
{
	importFile(db, fileName_);
}


