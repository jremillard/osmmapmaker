#include "osmdata.h"

#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <osmium/io/any_input.hpp>

// For assembling multipolygons
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>

// For the DynamicHandler class
#include <osmium/dynamic_handler.hpp>

// For the NodeLocationForWays handler
#include <osmium/handler/node_locations_for_ways.hpp>

// Allow any format of input files (XML, PBF, ...)
#include <osmium/io/any_input.hpp>

// For osmium::apply()
#include <osmium/visitor.hpp>

// For the location index. There are different types of indexes available.
// This will work for all input files keeping the index in memory.
#include <osmium/index/map/flex_mem.hpp>

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;




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
	osmium::io::File input_file(fileName.toStdString());

	// Configuration for the multipolygon assembler. Here the default settings
	// are used, but you could change multiple settings.
	osmium::area::Assembler::config_type assembler_config;

	// collect all relations and member ways needed for each area. 
	osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager(assembler_config);

	// We read the input file twice. In the first pass, only relations are
	// read and fed into the multipolygon manager.
	osmium::relations::read_relations(input_file, mp_manager);

	// The index storing all node locations.
	index_type index;

	// The handler that stores all node locations in the index and adds them
	// to the ways.
	location_handler_type location_handler{ index };

	// If a location is not available in the index, we ignore it. It might
	// not be needed (if it is not part of a multipolygon relation), so why
	// create an error?
	location_handler.ignore_errors();

	OsmDataImportHandler handler(db, dataName());

	// On the second pass we read all objects and run them first through the
	// node location handler and then the multipolygon collector. The collector
	// will put the areas it has created into the "buffer" which are then
	// fed through our "handler".
	osmium::io::Reader reader{ input_file };

	osmium::apply(reader, location_handler, handler);

	/* for areas from relations
	osmium::apply(reader, location_handler, mp_manager.handler([&handler](osmium::memory::Buffer&& buffer) {
		osmium::apply(buffer, handler);
	}));

	*/
	reader.close();


}


OsmDataImportHandler::OsmDataImportHandler(SQLite::Database &db, QString dataSource) : db_(db) , dataSource_(dataSource)
{
	QFile jsonFile(":/resources/presets.json");
	jsonFile.open(QFile::ReadOnly);
	QJsonDocument presetDoc = QJsonDocument::fromJson(jsonFile.readAll());

	QJsonObject presets = presetDoc.object();

	for (auto presetIter = presets.begin(); presetIter != presets.end(); ++presetIter)
	{
		QJsonObject preset = presetIter.value().toObject();

		QJsonArray geoTypes = preset.value("geometry").toArray();

		bool isArea = false;
		bool isLine = false;

		for (auto geo : geoTypes)
		{
			if (geo.toString() == "area")
				isArea = true;
			if (geo.toString() == "line")
				isLine = true;
		}

		if (isArea && isLine == false) // exclusive area's
		{
			QJsonObject tags = preset.value("tags").toObject();
			for (auto tagsIter = tags.begin(); tagsIter != tags.end(); ++tagsIter)
			{
				QString val = tagsIter.value().toString();
				QString key = tagsIter.key();
				if (key != "brand:wikidata") // wikidata isn't going to be a primary key.
				{
					if (val == "*")
						areas_.insert(key + QString(":::"), 0);
					else
						areas_.insert(key + QString(":::") + val, 0);
				}
			}
		}
	}

	queryAdd_ = new SQLite::Statement(db_, "INSERT INTO entity (type, source, geom) VALUES (?,?,?)");
	queryAddKV_ = new SQLite::Statement(db_, "INSERT INTO entityKV(id, key, value) VALUES (?,?,?)");

	{
		QFile file(":/resources/discarded.txt");
		bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
		assert(open);

		QTextStream in(&file);
		while (!in.atEnd())
		{
			QString line = in.readLine().trimmed();
			if (line.isEmpty() == false)
			{
				discardedKeys_.push_back(line.toStdString());
			}
		}
	}
}

OsmDataImportHandler::~OsmDataImportHandler()
{
	delete queryAdd_;
	delete queryAddKV_;
}


void OsmDataImportHandler::node(const osmium::Node& node)
{
	int tagCount = 0;

	for (const osmium::Tag &tag : node.tags())
	{
		bool discard = false;

		for (const std::string &discardKey : discardedKeys_)
		{
			if (std::strcmp(tag.key(), discardKey.c_str()) == 0)
			{
				discard = true;
				break;
			}
		}

		if  (discard == false)
			++tagCount;
	}

	if (tagCount > 0)
	{
		queryAdd_->bind(1, OET_POINT);
		queryAdd_->bind(2, dataSource_.toStdString());
		std::string pt = factory_.create_point(node);

		queryAdd_->bind(3, pt.c_str(), pt.size());

		queryAdd_->exec();

		long long entityId = db_.getLastInsertRowid();

		queryAdd_->reset();

		addTagsToDb(entityId, node.tags());
	}
	else
	{
		// empty nodes can't be rendered on a map.
	}
}

void OsmDataImportHandler::way(const osmium::Way& way)
{
	bool line = true;

	try
	{
		for (const osmium::Tag &tag : way.tags())
		{
			QString tagName = QString(tag.key()) + ":::";

			if (areas_.find(tagName) != areas_.end())
				line = false;

			tagName += tag.value();

			if (areas_.find(tagName) != areas_.end())
				line = false;
		}

		if (line)
		{
			queryAdd_->bind(1, OET_LINE);
			queryAdd_->bind(2, dataSource_.toStdString());
			std::string pt = factory_.create_linestring(way);
			queryAdd_->bind(3, pt.c_str(), pt.size());
		}
		else
		{
			queryAdd_->bind(1, OET_AREA);
			queryAdd_->bind(2, dataSource_.toStdString());
			std::string pt = factory_.create_linestring(way);
			queryAdd_->bind(3, pt.c_str(), pt.size());
		}

		queryAdd_->exec();

		long long entityId = db_.getLastInsertRowid();

		queryAdd_->reset();

		addTagsToDb(entityId, way.tags());
	}
	catch (osmium::geometry_error &)
	{
		// eat it.
	}

}

void OsmDataImportHandler::relation(const osmium::Relation& relation)
{
	bool line = true;

	try
	{
	}
	catch (osmium::geometry_error &)
	{
		// eat it.
	}
}

void OsmDataImportHandler::area(const osmium::Area& area)
{
	try
	{
		bool line = true;

		if (line == false)
		{
			queryAdd_->bind(1, OET_AREA);
			queryAdd_->bind(2, dataSource_.toStdString());
			std::string pt = factory_.create_multipolygon(area);
			queryAdd_->bind(3, pt.c_str(), pt.size());
			queryAdd_->exec();

			long long entityId = db_.getLastInsertRowid();

			queryAdd_->reset();

			addTagsToDb(entityId, area.tags());
		}
	}
	catch (osmium::geometry_error &)
	{
		// eat it.
	}
}

void OsmDataImportHandler::addTagsToDb(long long entityId, const osmium::TagList &tagList)
{
	for (const osmium::Tag &tag : tagList)
	{
		bool discard = false;

		for (const std::string &discardKey : discardedKeys_)
		{
			if (std::strcmp(tag.key(), discardKey.c_str()) == 0)
			{
				discard = true;
				break;
			}
		}

		if (discard == false)
		{
			queryAddKV_->bind(1, entityId);
			queryAddKV_->bind(2, tag.key());
			queryAddKV_->bind(3, tag.value());

			queryAddKV_->exec();
			queryAddKV_->reset();
		}
	}

}




