#pragma once

#include "datasource.h"

#include <osmium/handler.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/geom/wkb.hpp>
#include <unordered_set>
#include <geos_c.h>

/// Base class for all OSM based data sources.
///
/// Provides functionality to parse an OSM file and
/// store the entities into the project database.
class OsmData : public DataSource {
public:
    OsmData();
    OsmData(QDomElement projectNode);
    ~OsmData();

    void importFile(SQLite::Database& db, QString fileName);
    void importBuffer(SQLite::Database& db, const QByteArray& buffer);
};

/// osmium handler that imports nodes, ways and areas into SQLite.
class OsmDataImportHandler : public osmium::handler::Handler {
public:
    OsmDataImportHandler(SQLite::Database& db, QString dataSource);
    ~OsmDataImportHandler();

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

    SQLite::Database& db_;
};
