#pragma once

#include "osmdata.h"

#include <QDomNode>

/// Downloads OSM data directly from the OpenStreetMap servers.
///
/// This class currently only stores the configuration and shares
/// the import logic from OsmData.
class OsmDataDirectDownload : public OsmData {
public:
    OsmDataDirectDownload(QDomNode projectNode);
    ~OsmDataDirectDownload();

    void importData(RenderDatabase& db);

    virtual void saveXML(QDomDocument& doc, QDomElement& toElement);
};
