#pragma once

#include "datasource.h"

class OsmData : public DataSource
{
public:
	OsmData();
	~OsmData();

	void importFile(QString fileName);
};
