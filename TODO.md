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

## Administrative Boundary Rendering
### Goal
Support specialized styling and labeling of municipal, county and state
boundaries.

### Specification
1. Relations tagged `boundary=administrative` are imported as line entities and
   retain their `admin_level` tags.
2. Style layers can use `dashArray`, `color` and `label` settings to draw each
   level differently. Labels repeat along the boundary geometry.
3. Boundaries render after area fills but before road casings.
4. Sample projects include default styles for levels 4 (state), 6 (county) and
   8 (municipal).

### Automated Testing
- Unit test imports a sample administrative relation and confirms the line
  entity with the expected `admin_level` exists.
- Rendering tests ensure boundaries appear in the proper order with repeated
  labels.

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

### Specification
1. Provide a wizard for entering an Overpass query or selecting from templates
   (e.g. amenities, building outlines).
2. Download the results as `.osm` data and feed them into the existing import
   pipeline.
3. Cache query results during a session to avoid duplicate network requests and
   show progress with a cancel option.
4. Gracefully handle Overpass errors and document example queries in the user
   manual.

### Automated Testing
- Mock Overpass responses to test successful imports and error handling.
- Verify results are cached within a session and imported into the pipeline.

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

## Finalize XML Style Format
### Goal
Freeze the XML schema for style files so third‑party tools can rely on a stable
format.

### Specification
1. Audit all current style attributes and document them in
   `resources/project.xsd` with appropriate data types and defaults.
2. Add a `version` attribute to the `<style>` root element and update example
   styles accordingly.
3. Deprecate experimental attributes by providing clear replacements and supply
   XSLT scripts for migrating old files.
4. Update tests and sample projects to use the finalized schema and validate
   them during CI builds.

### Automated Testing
- Validate example style files against the new schema in unit tests.
- Ensure deprecated attributes are flagged with meaningful errors.


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


