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

## TODO

- output GUI, read zooms
- Create release build
- Wrap lines by word on lines
- Add text rendering priority
- Add support for other marker types for point features; currently only dots are supported
- Graphics fill on area type
- Rework the tag filter dialog box; it is confusing
- Tag filtering should use existing filters when filtering values
- Administrative boundary rendering via relations
- Import elevation data
- Directly import OSM data via Overpass query
- Golf style sheet
- Finalize XML style format
- Handle invalid field names [] in text boxes
- Fix stuck focus in zoom box
- Transform Road to Rd, Lane to Ln, Street to St, etc.
- Route transforms: make MA 119 display as Rt 119 in labels
- Alternate labels: [ref] and [name]; use semicolons
