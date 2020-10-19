# osmmapmaker
This is a desktop application for creating static maps directly using OpenStreetMap data. 

It aims to have a very simple workflow that non-engineers can used to create maps. The OSM data is 
directly imported, the user styles the map, and then generate the tiles all from within the
application without any other tools. 

It has its own style sheet system that is easier to use than the industrial grade mapnik xml, cartocss, 
or Mapbox GL JS. Because OpenStreetMap data is directly imported, without any filtering, all the data 
that is available in OpenStreetMap can be used in create maps. Therefor, specialty maps can be created 
directly from OpenStreetMap data. 

The dependencies include QT 5, BZip2, zlib, expat, Proj4, SQLiteCpp, sqlite3, geos, and osmium.

It is early days still, but to see its output, visit the www.grotontrails.org site and select
interactive maps.

TODO
- output GUI, read zooms
- Create release build.
- Wrap lines by word on lines.
- Add text rendering priority. 
- Add support for other markers types for point types, currently only dots are supported.
- Graphics fill on area type
- Port to Linux.
- Rework tag filter dialog box, it is confusing.
- Tag filtering should use existing filters when filter values.
- Admin boundary rendering via relations
- Import elevation data
- Directly import OSM data via overpass query
- Golf style sheet
- Finalize XML style format.
- Deal with invalid field names [] in text box.
- Stuck focus in zoom box.
- Transform Road to Rd, Lane to Ln, Street to St, etc.
- Route tranforms, make MA 119, say Rt 119 in label, 
- Do every other labels, [ref] and [name], semicolon. 
