#pragma once

#include "datasource.h"
#include "osmiumimporter.h"

#include <QDomNode>

/// Loads OSM data from a local file and imports it using
/// the common OsmData parsing logic.
class OsmDataFileSource : public DataSource {
public:
    OsmDataFileSource();
    OsmDataFileSource(QDomElement projectNode);

    ~OsmDataFileSource();

    void importData(RenderDatabase& db) override;

    QString localFile();
    void SetLocalFile(QString file);

    void saveXML(QDomDocument& doc, QDomElement& toElement) override;

private:
    QString fileName_;
};
