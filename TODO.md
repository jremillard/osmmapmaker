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
TBD

### Specification
TBD

## Additional Marker Types for Points
### Goal
TBD

### Specification
TBD

## Tag Filter Dialog Revamp
### Goal
TBD

### Specification
TBD

## Administrative Boundary Rendering
### Goal
TBD

### Specification
TBD

## Elevation Data Import
### Goal
TBD

### Specification
TBD

## Overpass Import Integration
### Goal
TBD

### Specification
TBD

## Golf Theme Stylesheet
### Goal
TBD

### Specification
TBD

## Finalize XML Style Format
### Goal
TBD

### Specification
TBD

## Handling Invalid Field Names
### Goal
TBD

### Specification
TBD

## Fix Stuck Focus in Zoom Box
### Goal
TBD

### Specification
TBD

## Alternate Labels with Semicolons
### Goal
TBD

### Specification
TBD

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

