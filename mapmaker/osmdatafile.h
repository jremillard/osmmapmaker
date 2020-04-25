#pragma once

#include "osmdata.h"

#include <QDomNode>

class OsmDataFile : public OsmData
{
public:
	OsmDataFile(QDomNode projectNode);
	~OsmDataFile();

	virtual void Import();
};







