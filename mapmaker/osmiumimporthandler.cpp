#include "osmiumimporthandler.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <geos.h>
#include <osmium/geom/wkb.hpp>
#include <sstream>

static const double detToM = 6356.0 * 1000.0 * 2.0 * (std::atan(1.0) * 4) / 360;

OsmiumImportHandler::OsmiumImportHandler(RenderDatabase& db, QString dataSource)
    : db_(db)
    , dataSource_(dataSource)
{
    // Read the JSON configuration that lists which key names imply that a way
    // should be treated as an area.  Some key/value pairs are explicitly
    // blacklisted because they represent linear features even though the key
    // name normally denotes an area.
    QFile jsonFile(":/resources/areaKeys.json");
    if (!jsonFile.open(QFile::ReadOnly)) {
        throw std::runtime_error(std::string("Cannot open resource: ") + jsonFile.fileName().toStdString());
    }
    QJsonDocument areaKeysDoc = QJsonDocument::fromJson(jsonFile.readAll());

    QJsonObject areaKey = areaKeysDoc.object().value("areaKeys").toObject();

    for (auto areaIter = areaKey.begin(); areaIter != areaKey.end(); ++areaIter) {
        // Each key in the JSON file becomes an "area key".  Any closed way with
        // that key is interpreted as an area unless a specific value is present
        // in the blacklist below.
        areaKeys_.insert(areaIter.key(), 0);

        QJsonObject blackList = areaIter.value().toObject();

        for (auto blackListIter = blackList.begin(); blackListIter != blackList.end(); ++blackListIter) {
            areaKeyValBlackList_.insert(areaIter.key() + ":::" + blackListIter.key(), 0);
        }
    }

    // "geom" stores the WKB blob created below.  The blob is in the same
    // WGS84 projection as the input data and can be read by GEOS or any other
    // OGC‑compatible parser.
    queryAdd_ = new SQLite::Statement(db_, "INSERT INTO entity (type, source, geom, linearLengthM, areaM) VALUES (?,?,?,?,?)");
    queryAddKV_ = new SQLite::Statement(db_, "INSERT INTO entityKV(id, key, value) VALUES (?,?,?)");
    queryAddSpatialIndex_ = new SQLite::Statement(db_, "INSERT INTO entitySpatialIndex(pkid, xmin, xmax,ymin, ymax) VALUES (?,?,?,?,?)");

    {
        // The discarded.txt resource lists keys that are ignored during import
        // because they do not carry rendering relevant information (for
        // example various source or attribution tags).  They are skipped both
        // when deciding if a node has useful tags and when storing tags in the
        // database.
        QFile discardedFile(":/resources/discarded.txt");
        if (!discardedFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error(std::string("Cannot open resource: ") + discardedFile.fileName().toStdString());
        }

        QTextStream in(&discardedFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() == false) {
                discardedKeys_.push_back(line.toStdString());
            }
        }
    }
}

OsmiumImportHandler::~OsmiumImportHandler()
{
    delete queryAdd_;
    delete queryAddKV_;
    delete queryAddSpatialIndex_;
}

void OsmiumImportHandler::node(const osmium::Node& node)
{
    // Nodes are only stored if they have at least one tag that is not in the
    // discard list.  This keeps the database small because the majority of OSM
    // nodes carry no interesting information.
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
            ++tagCount; // keep count of useful tags
    }

    if (tagCount > 0) {
        // Insert the geometry into the entity table as a point in WKB format
        // along with some precomputed metrics.
        queryAdd_->bind(1, OET_POINT);
        queryAdd_->bind(2, dataSource_.toStdString());
        std::string pt = factory_.create_point(node);

        queryAdd_->bind(3, pt.c_str(), pt.size());

        // Points have no length or area
        queryAdd_->bind(4, 0);
        queryAdd_->bind(5, 0);

        queryAdd_->exec();

        long long entityId = db_.getLastInsertRowid();

        queryAdd_->reset();

        addTagsToDb(entityId, node.tags());

        // Create a degenerate box to index the single point in the RTree.
        osmium::Location loc = node.location();
        osmium::Box bbBox(loc, loc);
        addSpatialIndexToDb(entityId, bbBox);
    } else {
        // Nodes without tags are skipped entirely as they carry no rendering
        // information.
    }
}

void OsmiumImportHandler::way(const osmium::Way& way)
{
    try {
        // Ways may represent either linear features or polygonal areas.  We
        // first decide which interpretation to use based on the tags.
        bool area = false;

        // Closed ways are potential areas.  Check whether their tags match the
        // area key list loaded earlier.
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

            // An explicit area=yes/area=no tag overrides the heuristic above.
            for (const osmium::Tag& tag : way.tags()) {
                if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "yes") == 0)
                    area = true;
                else if (strcmp(tag.key(), "area") == 0 && strcmp(tag.value(), "no") == 0)
                    area = false;
            }
        }

        if (area == false) {
            // Treat as a line geometry
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
        // Invalid geometries are simply skipped.  They are rare and usually
        // indicate bad source data.
    }
}

void OsmiumImportHandler::relation(const osmium::Relation& relation)
{
}

void OsmiumImportHandler::area(const osmium::Area& area)
{
    try {
        // Areas are generated by the multipolygon assembler and represent
        // closed polygons with tags merged from their relation and member ways.
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
            // Store either as an area or as a closed line depending on the
            // heuristics above.
            if (addArea)
                queryAdd_->bind(1, OET_AREA);
            else
                queryAdd_->bind(1, OET_LINE);

            queryAdd_->bind(2, dataSource_.toStdString());
            // Convert the assembled area to WKB and store it
            std::string wkbBuffer = factory_.create_multipolygon(area);
            queryAdd_->bind(3, wkbBuffer.c_str(), wkbBuffer.size());

            geos::io::WKBReader geomFactory;

            std::istringstream strStr(wkbBuffer);

            std::unique_ptr<geos::geom::Geometry> geom = geomFactory.read(strStr);

            // Convert perimeter and area from degrees to meters for convenient
            // querying later on.
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
            // The multipolygon did not result in a valid geometry.  Ignore it.
        }
    } catch (osmium::geometry_error&) {
        // Skip areas for which the geometry library fails to produce output.
    }
}

void OsmiumImportHandler::addSpatialIndexToDb(long long entityId, const osmium::Box& bbBox)
{
    // Insert a bounding box for the geometry into the RTree index.  Only the
    // minimum rectangle is stored as the application does not need complex
    // containment queries.

    queryAddSpatialIndex_->bind(1, static_cast<int64_t>(entityId));

    queryAddSpatialIndex_->bind(2, bbBox.bottom_left().lon_without_check());
    queryAddSpatialIndex_->bind(3, bbBox.top_right().lon_without_check());

    queryAddSpatialIndex_->bind(4, bbBox.bottom_left().lat_without_check());
    queryAddSpatialIndex_->bind(5, bbBox.top_right().lat_without_check());

    queryAddSpatialIndex_->exec();
    queryAddSpatialIndex_->reset();
}

void OsmiumImportHandler::addTagsToDb(long long entityId, const osmium::TagList& tagList)
{
    // Store all tags except those on the discard list for the given entity id.
    for (const osmium::Tag& tag : tagList) {
        bool discard = false;

        for (const std::string& discardKey : discardedKeys_) {
            if (std::strcmp(tag.key(), discardKey.c_str()) == 0) {
                discard = true;
                break;
            }
        }

        if (!discard) {
            queryAddKV_->bind(1, static_cast<int64_t>(entityId));
            queryAddKV_->bind(2, tag.key());
            queryAddKV_->bind(3, tag.value());
            queryAddKV_->exec();
            queryAddKV_->reset();
        }
    }
}
