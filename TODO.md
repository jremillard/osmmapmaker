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

## Notes
- This replaces the old TODO items about road name transformation and route
  renaming that were previously listed in `README.md`.

## Word-based Line Wrapping
### Goal
Allow label text to break at word boundaries instead of arbitrary character
positions so that multi line labels remain legible on narrow features.

### Specification
1. Add an optional `wordWrap="true"` attribute on `<label>` elements.
2. When enabled, determine the maximum line width using the label's bounding
   box and font metrics.
3. Use `QTextBoundaryFinder` to iterate over Unicode word breaks and build lines
   that fit within the width.  Hyphenate only when a single word exceeds the
   width.
4. Update `RenderQT` and `TextFieldProcessor` to respect the new attribute.
5. Create unit tests verifying wrapped output for long road names and points of
   interest.

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

## Administrative Boundary Rendering
### Goal
Support specialized styling and labeling of municipal, county and state
boundaries.

### Specification
1. Import `boundary=administrative` relations along with their `admin_level`
   tags during OSM loading.
2. Add style properties for dashed line patterns, colors and label text so each
   admin level can be drawn distinctly.
3. Ensure boundaries render above area fills but below road casings and that
   labels follow the boundary line with repeat intervals.
4. Provide default style snippets for common levels and include examples in the
   sample projects.

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

## Handling Invalid Field Names
### Goal
Prevent crashes when project files reference invalid or unknown field names.

### Specification
1. Validate field names during project load against the list of allowed tags and
   characters.
2. Skip invalid fields while logging a warning so the rest of the project
   continues to load.
3. Highlight problematic entries in the GUI with a tooltip explaining the
   issue.
4. Sanitize field names used in generated SQL to avoid quoting errors and add
   tests that open projects containing bad fields.

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

