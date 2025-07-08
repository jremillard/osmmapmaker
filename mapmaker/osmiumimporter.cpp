#include "osmiumimporter.h"
#include "osmiumimporthandler.h"
#include <QTemporaryFile>
#include <osmium/io/any_input.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/visitor.hpp>

using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

void OsmiumImporter::importFile(RenderDatabase& db, const QString& fileName, const QString& dataSource)
{
    osmium::io::File input_file(fileName.toStdString());
    osmium::area::Assembler::config_type assembler_config;
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager(assembler_config);
    osmium::relations::read_relations(input_file, mp_manager);
    index_type index;
    location_handler_type location_handler { index };
    location_handler.ignore_errors();
    OsmiumImportHandler handler(db, dataSource);
    osmium::io::Reader reader { input_file };
    osmium::apply(reader, location_handler, handler, mp_manager.handler([&handler](osmium::memory::Buffer&& buffer) {
        osmium::apply(buffer, handler);
    }));
    reader.close();
}

void OsmiumImporter::importBuffer(RenderDatabase& db, const QByteArray& buffer, const QString& dataSource)
{
    QTemporaryFile tmp("XXXXXX.osm");
    tmp.open();
    tmp.write(buffer);
    tmp.flush();
    tmp.close();
    importFile(db, tmp.fileName(), dataSource);
}
