#pragma once

#include "osmdata.h"

#include <QDomNode>

/// Loads OSM data from a local file and imports it using
/// the common OsmData parsing logic.
class OsmDataFile : public OsmData {
public:
    OsmDataFile();
    OsmDataFile(QDomElement projectNode);

    ~OsmDataFile();

    void importData(RenderDatabase& db);

    QString localFile();
    void SetLocalFile(QString file);

    virtual void saveXML(QDomDocument& doc, QDomElement& toElement);

private:
    QString fileName_;
};
