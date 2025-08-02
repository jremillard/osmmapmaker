#osmmapmaker

osmmapmaker is a desktop application for creating static maps directly from OpenStreetMap data. It aims to be simple enough that non-engineers can import OSM data, style their maps, and generate the tiles from within the application.

The program includes its own stylesheet system, which is easier to use than Mapnik XML, CartoCSS, or Mapbox GL JS. Because the data is imported without filtering, any information available in OpenStreetMap can be used to build specialized maps.

To see example output, visit [grotontrails.org](http://www.grotontrails.org) and choose the interactive maps.

The application accepts a `--project` command line option to open a project file directly. If this option is omitted, a file picker will be shown on startup so you can choose the project to load.

An optional `--output` argument can be used to generate map outputs without launching the UI. When invoked as `--output` all configured outputs are written. When invoked as `--output <name>` only the specified output is generated.

## Dependencies

The project relies on the following libraries:

- Qt 5 (Widgets, Core, Xml, and XmlPatterns)
- BZip2
- Zlib
- Expat
- Proj4
- SQLiteCpp
- SQLite3
- GEOS
- Libosmium
- Catch2

Development and testing also require `clang-format`, `lcov`, and `valgrind`.

On Debian-based systems you can install them with `install_dependencies_apt.sh`.

See the AGENTS.MD file for detailed instructions on building the project.