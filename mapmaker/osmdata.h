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

class OsmData : public DataSource
{
public:
	OsmData();
	OsmData(QDomElement projectNode);
	~OsmData();

	void importFile(SQLite::Database &db, QString fileName);
};

class OsmDataImportHandler : public osmium::handler::Handler
{
public:
	OsmDataImportHandler(SQLite::Database &db, QString dataSource);
	~OsmDataImportHandler();

	void node(const osmium::Node& node);
	void way(const osmium::Way& way);
	void relation(const osmium::Relation& relation);
	void area(const osmium::Area& area);

private:
	osmium::geom::WKBFactory<osmium::geom::IdentityProjection> factory_; 

	QString dataSource_;
	SQLite::Statement *queryAddPoint_;
	SQLite::Statement *queryAddPointKV_;

	SQLite::Database &db_;
};

