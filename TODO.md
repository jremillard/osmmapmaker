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

## Output GUI Zoom Controls
### Goal
TBD

### Specification
TBD

## Word-based Line Wrapping
### Goal
TBD

### Specification
TBD

## Text Rendering Priority System
### Goal
TBD

### Specification
TBD

## Additional Marker Types for Points
### Goal
TBD

### Specification
TBD

## Area Graphics Fill
### Goal
TBD

### Specification
TBD

## Tag Filter Dialog Revamp
### Goal
TBD

### Specification
TBD

## Tag Filtering Reuse Existing Filters
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

## Road Name Abbreviations
### Goal
TBD

### Specification
TBD

## Route Name Shortening
### Goal
TBD

### Specification
TBD

## Alternate Labels with Semicolons
### Goal
TBD

### Specification
TBD

