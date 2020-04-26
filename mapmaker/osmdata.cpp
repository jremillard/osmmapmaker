#include "osmdata.h"

#include <osmium/io/any_input.hpp>

OsmData::OsmData()
{
}

OsmData::OsmData(QDomElement projectNode) : DataSource( projectNode)
{

}

OsmData::~OsmData()
{
}


void OsmData::importFile(SQLite::Database &db, QString fileName)
{
	// The Reader is initialized here with an osmium::io::File, but could

	// also be directly initialized with a file name.


	osmium::io::File input_file( fileName.toStdString() );

	osmium::io::Reader reader(input_file);

	OsmDataImportHandler handler(db, dataName());
	osmium::apply(reader, handler);

	//osmium::geom::WKTFactory<> factory;
	//osmium::geom::WKTFactory<> factory{ 3 }; // three digits after decimal point

	//osmium::geom::detail::WKTFactoryImpl t;

	//osmium::geom::GeometryFactory<, IdentityProjection> f;

	reader.close();
}


OsmDataImportHandler::OsmDataImportHandler(SQLite::Database &db, QString dataSource) : db_(db) , dataSource_(dataSource)
{
	queryAddPoint_ = new SQLite::Statement(db_, "INSERT INTO point(source, geom) VALUES (?,?)");
	queryAddPointKV_ = new SQLite::Statement(db_, "INSERT INTO pointKV(id, key, value) VALUES (?,?,?)");

}

OsmDataImportHandler::~OsmDataImportHandler()
{
	delete queryAddPoint_;
	delete queryAddPointKV_;
}


void OsmDataImportHandler::node(const osmium::Node& node)
{
	queryAddPoint_->bind(1, dataSource_.toStdString());
	std::string pt = factory_.create_point(node);
		
	queryAddPoint_->bind(2, pt.c_str(), pt.size());

	queryAddPoint_->exec();
	queryAddPoint_->reset();
}

void OsmDataImportHandler::way(const osmium::Way& way)
{
}

void OsmDataImportHandler::relation(const osmium::Relation& relation)
{
}

void OsmDataImportHandler::area(const osmium::Area& area)
{
}
