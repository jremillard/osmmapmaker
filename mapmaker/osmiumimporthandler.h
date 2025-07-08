#pragma once

#include "renderdatabase.h"
#include "datasource.h"
#include <osmium/geom/wkb.hpp>
#include <osmium/osm/box.hpp>
#include <osmium/handler.hpp>
#include <QHash>
#include <QString>
#include <vector>
#include <string>

class OsmiumImportHandler : public osmium::handler::Handler {
public:
    OsmiumImportHandler(RenderDatabase& db, QString dataSource);
    ~OsmiumImportHandler();

    void node(const osmium::Node& node);
    void way(const osmium::Way& way);
    void relation(const osmium::Relation& relation);
    void area(const osmium::Area& area);

private:
    void addTagsToDb(long long id, const osmium::TagList&);
    void addSpatialIndexToDb(long long entityId, const osmium::Box& bbBox);

    osmium::geom::WKBFactory<osmium::geom::IdentityProjection> factory_;
    std::vector<std::string> discardedKeys_;
    QHash<QString, int> areaKeys_;
    QHash<QString, int> areaKeyValBlackList_;
    QString dataSource_;
    SQLite::Statement* queryAdd_;
    SQLite::Statement* queryAddKV_;
    SQLite::Statement* queryAddSpatialIndex_;

    RenderDatabase& db_;
};
