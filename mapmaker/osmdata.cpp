#include "osmdata.h"

#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>

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

#include <geos.h>

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

static const double detToM = 6356.0 * 1000.0 * 2.0 * (std::atan(1.0) * 4) / 360;

OsmData::OsmData()
{
}

OsmData::OsmData(QDomElement projectNode)
    : DataSource(projectNode)
{
}

OsmData::~OsmData()
{
}

void OsmData::importFile(RenderDatabase& db, QString fileName)
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
    location_handler_type location_handler { index };

    // If a location is not available in the index, we ignore it. It might
    // not be needed (if it is not part of a multipolygon relation), so why
    // create an error?
    location_handler.ignore_errors();

    OsmDataImportHandler handler(db, dataName());

    // On the second pass we read all objects and run them first through the
    // node location handler and then the multipolygon collector. The collector
    // will put the areas it has created into the "buffer" which are then
    // fed through our "handler".
    osmium::io::Reader reader { input_file };

    // osmium::apply(reader, location_handler, handler);

    // for areas from relations
    osmium::apply(reader, location_handler, handler, mp_manager.handler([&handler](osmium::memory::Buffer&& buffer) {
        osmium::apply(buffer, handler);
    }));

    reader.close();
}

void OsmData::importBuffer(RenderDatabase& db, const QByteArray& buffer)
{
    QTemporaryFile tmp("XXXXXX.osm");
    tmp.open();
    tmp.write(buffer);
    tmp.flush();
    tmp.close();
    importFile(db, tmp.fileName());
}

OsmDataImportHandler::OsmDataImportHandler(RenderDatabase& db, QString dataSource)
    : db_(db)
    , dataSource_(dataSource)
{
    QFile jsonFile(":/resources/areaKeys.json");
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument areaKeysDoc = QJsonDocument::fromJson(jsonFile.readAll());

    QJsonObject areaKey = areaKeysDoc.object().value("areaKeys").toObject();

    for (auto areaIter = areaKey.begin(); areaIter != areaKey.end(); ++areaIter) {
        areaKeys_.insert(areaIter.key(), 0);

        QJsonObject blackList = areaIter.value().toObject();

        for (auto blackListIter = blackList.begin(); blackListIter != blackList.end(); ++blackListIter) {
            areaKeyValBlackList_.insert(areaIter.key() + ":::" + blackListIter.key(), 0);
        }
    }

    queryAdd_ = new SQLite::Statement(db_, "INSERT INTO entity (type, source, geom, linearLengthM, areaM) VALUES (?,?,?,?,?)");
    queryAddKV_ = new SQLite::Statement(db_, "INSERT INTO entityKV(id, key, value) VALUES (?,?,?)");
    queryAddSpatialIndex_ = new SQLite::Statement(db_, "INSERT INTO entitySpatialIndex(pkid, xmin, xmax,ymin, ymax) VALUES (?,?,?,?,?)");

    {
        QFile file(":/resources/discarded.txt");
        bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
        assert(open);

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() == false) {
                discardedKeys_.push_back(line.toStdString());
            }
        }
    }
}

OsmDataImportHandler::~OsmDataImportHandler()
{
    delete queryAdd_;
    delete queryAddKV_;
    delete queryAddSpatialIndex_;
}

void OsmDataImportHandler::node(const osmium::Node& node)
{
    int tagCount = 0;

    for (const osmium::Tag& tag : node.tags()) {
        bool discard = false;

        for (const std::string& discardKey : discardedKeys_) {
            if (std::strcmp(tag.key(), discardKey.c_str()) == 0) {
                discard = true;
                break;
            }
        }

        if (discard == false)
            ++tagCount;
    }

    if (tagCount > 0) {
        queryAdd_->bind(1, OET_POINT);
        queryAdd_->bind(2, dataSource_.toStdString());
        std::string pt = factory_.create_point(node);

        queryAdd_->bind(3, pt.c_str(), pt.size());

        queryAdd_->bind(4, 0); //
        queryAdd_->bind(5, 0);

        queryAdd_->exec();

        long long entityId = db_.getLastInsertRowid();

        queryAdd_->reset();

        addTagsToDb(entityId, node.tags());

        osmium::Location loc = node.location();

        osmium::Box bbBox(loc, loc);
        addSpatialIndexToDb(entityId, bbBox);
    } else {
        // empty nodes can't be rendered on a map.
    }
}

void OsmDataImportHandler::way(const osmium::Way& way)
{
    try {
        bool area = false;

        if (way.is_closed()) {
            for (const osmium::Tag& tag : way.tags()) {
                QString keyName = QString(tag.key());

                if (areaKeys_.find(keyName) != areaKeys_.end()) {
                    area = true;

                    QString keyVal = QString(tag.key()) + ":::" + QString(tag.value());

                    if (areaKeyValBlackList_.find(keyVal) != areaKeyValBlackList_.end())
                        area = false;
                }
            }

            // area tag overrides everything
            for (const osmium::Tag& tag : way.tags()) {
                if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "yes") == 0)
                    area = true;
                else if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "no") == 0)
                    area = false;
            }
        }

        if (area == false) {
            queryAdd_->bind(1, OET_LINE);

            queryAdd_->bind(2, dataSource_.toStdString());

            std::string wkbBuffer = factory_.create_linestring(way);
            queryAdd_->bind(3, wkbBuffer.c_str(), wkbBuffer.size());

            geos::io::WKBReader geomFactory;

            std::istringstream strStr(wkbBuffer);

            std::unique_ptr<geos::geom::Geometry> geom = geomFactory.read(strStr);

            double length = geom->getLength();
            queryAdd_->bind(4, length * detToM);

            double area = 0;
            queryAdd_->bind(5, area);

            queryAdd_->exec();

            long long entityId = db_.getLastInsertRowid();

            queryAdd_->reset();

            addTagsToDb(entityId, way.tags());

            osmium::Box bbBox = way.envelope();
            addSpatialIndexToDb(entityId, bbBox);
        }
    } catch (osmium::geometry_error&) {
        // eat it.
    }
}

void OsmDataImportHandler::relation(const osmium::Relation& relation)
{
}

void OsmDataImportHandler::area(const osmium::Area& area)
{
    try {
        bool addArea = false;

        for (const osmium::Tag& tag : area.tags()) {
            QString keyName = QString(tag.key());

            if (areaKeys_.find(keyName) != areaKeys_.end()) {
                addArea = true;

                QString keyVal = QString(tag.key()) + ":::" + QString(tag.value());

                if (areaKeyValBlackList_.find(keyVal) != areaKeyValBlackList_.end())
                    addArea = false;
            }
        }

        // area tag overrides everything
        for (const osmium::Tag& tag : area.tags()) {
            if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "yes") == 0)
                addArea = true;
            else if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "no") == 0)
                addArea = false;
        }

        if (addArea) {
            if (addArea)
                queryAdd_->bind(1, OET_AREA);
            else
                queryAdd_->bind(1, OET_LINE);

            queryAdd_->bind(2, dataSource_.toStdString());
            std::string wkbBuffer = factory_.create_multipolygon(area);
            queryAdd_->bind(3, wkbBuffer.c_str(), wkbBuffer.size());

            geos::io::WKBReader geomFactory;

            std::istringstream strStr(wkbBuffer);

            std::unique_ptr<geos::geom::Geometry> geom = geomFactory.read(strStr);

            double lengthDeg = geom->getLength();
            queryAdd_->bind(4, lengthDeg * detToM);

            double areaDegSq = geom->getArea();
            queryAdd_->bind(5, areaDegSq * detToM * detToM);

            queryAdd_->exec();

            long long entityId = db_.getLastInsertRowid();

            queryAdd_->reset();

            addTagsToDb(entityId, area.tags());

            osmium::Box bbBox = area.envelope();
            addSpatialIndexToDb(entityId, bbBox);
        } else {
            // not sure what is going on with this.
        }
    } catch (osmium::geometry_error&) {
        // eat it.
    }
}

void OsmDataImportHandler::addSpatialIndexToDb(long long entityId, const osmium::Box& bbBox)
{
    // We don't do "contained within" kind of query's, don't need to expend it.

    queryAddSpatialIndex_->bind(1, static_cast<int64_t>(entityId));

    queryAddSpatialIndex_->bind(2, bbBox.bottom_left().lon_without_check());
    queryAddSpatialIndex_->bind(3, bbBox.top_right().lon_without_check());

    queryAddSpatialIndex_->bind(4, bbBox.bottom_left().lat_without_check());
    queryAddSpatialIndex_->bind(5, bbBox.top_right().lat_without_check());

    queryAddSpatialIndex_->exec();
    queryAddSpatialIndex_->reset();
}

void OsmDataImportHandler::addTagsToDb(long long entityId, const osmium::TagList& tagList)
{
    for (const osmium::Tag& tag : tagList) {
        bool discard = false;

        for (const std::string& discardKey : discardedKeys_) {
            if (std::strcmp(tag.key(), discardKey.c_str()) == 0) {
                discard = true;
                break;
            }
        }

        if (discard == false) {
            queryAddKV_->bind(1, static_cast<int64_t>(entityId));
            queryAddKV_->bind(2, tag.key());
            queryAddKV_->bind(3, tag.value());

            queryAddKV_->exec();
            queryAddKV_->reset();
        }
    }
}
