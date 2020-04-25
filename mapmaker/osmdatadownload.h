#pragma once

#include "osmdata.h"

#include <QDomNode>

class OsmDataDownload : public OsmData
{
public:
	OsmDataDownload(QDomNode projectNode);
	~OsmDataDownload();

	virtual void import();

};





