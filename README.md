# osmmapmaker

osmmapmaker is a desktop application for creating static maps directly from OpenStreetMap data. It aims to be simple enough that non-engineers can import OSM data, style their maps, and generate the tiles from within the application.

The program includes its own stylesheet system, which is easier to use than Mapnik XML, CartoCSS, or Mapbox GL JS. Because the data is imported without filtering, any information available in OpenStreetMap can be used to build specialized maps.

To see example output, visit [grotontrails.org](http://www.grotontrails.org) and choose the interactive maps.

## Dependencies

The project relies on the following libraries:

- Qt 5 (Widgets, Core, and Xml)
- BZip2
- Zlib
- Expat
- Proj4
- SQLiteCpp
- SQLite3
- GEOS
- Libosmium
- Catch2

On Debian-based systems you can install them with `install_dependencies.sh`.
