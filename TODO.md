# Named Abbreviation Sets for Labels

This feature will allow projects to define multiple text abbreviation sets within
project XML. Each set has a unique name and contains word or phrase mappings
used to shorten labels when rendering maps.

## Goals
- Support different abbreviation tables for roads, routes, or other contexts.
- Allow label styles to specify which abbreviation set to apply.
- Provide an XML schema for defining these named sets.

## Specification
1. **Project XML changes**
   - Add `<abbreviationSet name="road">` elements as children of the project
     root. Each set contains `<map from="Street" to="St"/>` style entries.
   - Multiple sets may be defined; names must be unique.

2. **Project API**
   - Load abbreviation sets into a dictionary keyed by the set name.
   - Expose a method to retrieve a specific set when rendering labels.

3. **Style layer syntax**
   - Extend text label style with an optional `abbrevSet` attribute that refers
     to a defined set name. Example:
     ```xml
     <label text="{name}" abbrevSet="road"/>
     ```
   - If not provided, labels are rendered without abbreviation.

4. **Route renaming**
   - The same mechanism should support route name shortcuts, such as converting
     `Massachusetts Route 119` to `MA 119` using a separate `route` set.

5. **Text processing**
   - When rendering a label with `abbrevSet`, look up each word in the chosen set
     and replace it if a mapping exists. Preserve case for capitalized words.

## Automated Testing
- Add unit tests that load a project with multiple abbreviation sets.
- Verify labels using `abbrevSet` render the expected text.


## Additional Marker Types for Points
### Goal
Provide more built‑in symbol shapes for point features so maps can represent a
variety of POI types without custom images.

### Specification
1. Extend style syntax with a `marker` attribute accepting values such as
   `circle`, `square`, `triangle`, `diamond`, `cross` and `star`.
2. Implement each shape using `QPainterPath` in the point rendering code and
   size them according to the existing `symbolSize` property.
3. Support custom images via `image="path"` and load them from Qt resources or
   the project directory.
4. Update the style editor UI to choose marker type or browse for an image.
5. Add schema validation and regression tests to confirm markers render
   correctly at multiple DPI levels.

### Automated Testing
- Render each marker shape at multiple sizes and compare against reference images.
- Check that custom image markers load correctly from project paths.

## Tag Filter Dialog Revamp
### Goal
Redesign the tag filter dialog to make it easier to build complex filters and to
save commonly used presets.

### Specification
1. Replace the single list widget with a table where each row contains a tag key
   and value along with check boxes for enabling or negating the row.
2. Provide buttons for adding, editing and removing rows directly in the dialog.
3. Allow saving the current filter as a named preset and selecting from existing
   presets via a drop‑down.
4. Display the number of matching objects in real time by running a background
   SQL query.
5. Persist filters in the project XML using a simple expression syntax and add
   tests for correct serialization.

### Automated Testing
- Programmatically modify rows in the filter dialog and ensure serialized filters match expectations.
- Verify preset saving and loading works across sessions.

## Elevation Data Import
### Goal
Allow projects to include digital elevation models (DEMs) so that hillshading
and contour lines can be generated.

### Specification
1. Accept GeoTIFF or IMG files and store them under a new `elevation` folder in
   the project.
2. Build a GDAL raster index on import and record the height range for each
   file.
3. Expose GUI options to enable hillshading and to create contour lines at
   user‑defined intervals.
4. Cache reprojected raster tiles for faster rendering.
5. Allow configuration of light angle and shading color per project and add
   tests that sample heights correctly.

### Automated Testing
- Import a small DEM file and verify hillshading and contour generation.
- Ensure cached raster tiles and height ranges are stored correctly.

## Overpass Import Integration
### Goal
Fetch OSM features directly from the Overpass API without requiring external
downloads.

### As Built
Implemented a new `OsmDataOverpass` data source. The class posts the user
defined query to the Overpass interpreter and imports the returned `.osm`
payload through the existing `OsmData` parser. Results are stored in a static
cache keyed by query so repeat imports during a session do not trigger network
access. If the network is unavailable or Overpass returns an error the importer
throws a runtime exception.

### Next Steps
- Add an import wizard that assists users in constructing queries.
  - The first templates should download by bounding box or by administrative
    boundary.

### Automated Testing
- Added `overpass_test` which seeds the cache with a small XML snippet and
  verifies the importer inserts entities without contacting the network.
- A second test confirms an exception is raised when the network is disabled and
  no cached result exists.

## Golf Theme Stylesheet
### Goal
Offer a prebuilt stylesheet optimized for rendering golf course maps.

### Specification
1. Create `golf.xml` defining colors and symbology for fairways, greens,
   bunkers, tees and hazards.
2. Include point icons for tee boxes and hole numbers and specify recommended
   layer ordering with existing road and water styles.
3. Ship a sample project that uses the theme with a real course dataset.
4. Validate the theme by rendering at multiple zoom levels and adjusting colors
   for print and screen output.

### Automated Testing
- Render example maps with the golf theme at several zoom levels and compare against reference images.
- Check that icons and colors match the specification.

## Fix Stuck Focus in Zoom Box
### Goal
Resolve an issue where keyboard focus does not return to the map view after
dragging a zoom selection box.

### Specification
1. Audit the mouse release handling in `RenderQT::SetupZoomBoundingBox` and
   ensure the rubber band object is deleted when the drag completes.
2. Explicitly call `setFocus()` on the main view after zooming and add a timeout
   fallback that cancels the mode if the release event is missed.
3. Create an automated test that simulates drag‑zoom to verify focus is
   restored.

### Automated Testing
- Simulate drag-zoom interactions and confirm the map view regains focus.
- Verify fallback timeout cancels zoom mode when mouse release is missed.

## Alternate Labels with Semicolons
### Goal
Use semicolon‑separated name variants so shorter labels can appear at low zoom
levels.

### Specification
1. Split the `{name}` field on semicolons during label processing and choose the
   first variant that fits within the current zoom‑dependent width.
2. Enable the feature with a `alternateNames="true"` attribute on `<label>`
   elements.
3. Apply abbreviation sets and case preservation to the chosen variant.
4. Provide unit tests covering names with multiple semicolons and ensuring the
   fallback behaves as expected.
5. Example tag values using semicolons in OSM include:
   - `name=Bundesautobahn 2;A2` for roads with both long and short forms.
   - `name=Rue de la Paix;Peace Street` demonstrating bilingual labeling.
   - `alt_name=I 495;Capital Beltway` for alternate highway names.

### Automated Testing
- Render names with multiple semicolons at varying widths to ensure the correct variant is chosen.
- Confirm abbreviation sets apply before variant fallback.

## RenderQT Unit Testing
### Goal
Automate verification of the Qt renderer so that geometry and label output stay
consistent across code changes.

### Specification
1. Create `tests/renderqt_data` containing:
   - minimal project XML files and style definitions
   - SQL or OSM snippets used to populate geometry tables
   - benchmark PNG images for comparison
2. Each test should:
   - load sample data into an in-memory SQLite database
   - construct a `Project` instance using the sample XML
   - configure `RenderQT` with `SetupZoomAtCenter` or `SetupZoomBoundingBox`
   - render an image and compare every pixel to the benchmark PNG with a small
     tolerance
3. Provide helper functions to build the database and load the expected images.
4. Test cases derived from `renderqt.cpp` include:
   - single point rendering with varying symbol sizes
   - lines with width, casing and dash patterns
   - area fills with casing
   - label placement for points and lines including halos and word wrapping
   - zoom-based visibility using `minZoom` values
   - background color fill and bounding box transforms at multiple DPI scales
5. Run tests at 1x and 2x DPI to ensure scaling logic works.
### Automated Testing
- Integrate the RenderQT image comparison tests into the CTest suite for CI.
- Provide scripts to generate benchmarks and fail when pixels differ beyond tolerance.

## Project New
### Goal
Allow creation of new empty projects directly from the main application.

### Specification
1. Provide a dialog or wizard launched from the File menu to choose a destination path and default style.
2. Generate the project XML and folder layout automatically so users can begin importing data immediately.
3. Offer templates for common project types with preconfigured layers.

### Automated Testing
- Confirm the wizard creates valid project files that load without errors.

## Project Copy
### Goal
Duplicate an existing project to a different location for experimentation.

### Specification
1. Add a `Copy Project` command that prompts for the target directory.
2. Copy the project file and all referenced resources such as stylesheets and icons.
3. Update any relative paths so the copy renders identically.

### Automated Testing
- Verify the copied project opens correctly and produces the same output as the original.

## Daily Logging to File
### Goal
Capture trace output in a rotating log so issues can be diagnosed easily.

### Specification
1. Use the header-only [spdlog](https://github.com/gabime/spdlog) library for cross‑platform logging.
2. Create a daily logger that writes to `Preferences::logDirectory()/osmmapmaker.log` so the location can be customized by the user.
3. Rotate (delete) the previous log at startup so each run begins fresh.
4. Provide a small wrapper that initializes the logger and exposes simple `log::info` style functions.
5. Allow logging to be disabled in release builds via a compile flag.

### Automated Testing
- Launch the application twice and verify the log file from the first run is removed before the second begins.
- Run the unit test suite and confirm no log file is created.

## detToM Conversion Accuracy
### Goal
Investigate whether the constant converting degrees to meters in `osmdata.cpp` uses the correct Earth radius.

### Background
Codex flagged a discrepancy between the implementation:
```cpp
static const double detToM = 6356.0 * 1000.0 * 2.0 * (std::atan(1.0) * 4) / 360;
```
and a comment that references the WGS84 equatorial radius:
```cpp
static const double detToM = 6378137.0 * M_PI / 180.0;
```
Using the truncated radius could underestimate lengths.

### Specification
1. Confirm which value is intended and ensure the constant is defined only once.
2. Update related calculations and documentation to use the chosen radius consistently.

### Automated Testing
- Run existing length and area computation tests to verify results remain correct after any change.
