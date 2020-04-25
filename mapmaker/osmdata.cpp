#include "osmdata.h"

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/osm/node.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/visitor.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

OsmData::OsmData()
{
}

OsmData::~OsmData()
{
}

class MyHandler : public osmium::handler::Handler 
{
public:
	int count = 0;

	void node(const osmium::Node& node)
	{
		++count;
	}

	void way(const osmium::Way& way) 
	{
		++count;

	}

	void relation(const osmium::Relation& relation) 
	{
		++count;
	}

	void area(const osmium::Area& area) 
	{
		++count;
	}

};

void OsmData::importFile( QString fileName)
{
	// The Reader is initialized here with an osmium::io::File, but could

	// also be directly initialized with a file name.


	osmium::io::File input_file( fileName.toStdString() );

	osmium::io::Reader reader(input_file);


	MyHandler handler;
	osmium::apply(reader, handler);

	reader.close();
}


