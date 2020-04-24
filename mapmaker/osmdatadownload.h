#pragma once

#include "datasource.h"

#include <QDomNode>

class OsmDataDownload : public dataSource
{
public:
	OsmDataDownload(QDomNode projectNode);
	~OsmDataDownload();
};





