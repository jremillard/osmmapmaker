#osmmapmaker

osmmapmaker is a desktop application for cre### Build

Build each directory with `cmake --build bin/<type> -j$(nproc)`. maps directly from OpenStreetMap data. It aims to be simple enough that non-engineers can import OSM data, style their maps, and generate the tiles from within the application.

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

## Building the Project

### Windows Build Setup

1. Open the Visual Studio 2022 Command Prompt (Developer Command Prompt for VS 2022).
2. Check if `vcpkg` is installed by verifying the `VCPKG_ROOT` environment variable: `if exist %VCPKG_ROOT% (echo vcpkg is installed) else (echo vcpkg is not installed)`.
3. Install packages `vcpkg install`

### Linux/macOS Build Setup

1. Install required packages using your distribution's package manager:
   - Ubuntu/Debian: `sudo apt-get install` followed by the packages in `install_dependencies_apt.sh`.
   - Other distributions: Install equivalent packages.
2. Note: The `vcpkg.json` file is for Windows builds only and is not used on Linux/macOS systems.

### Configure CMake Build Directories

1. Delete the `bin` directory if it exists.

**For Windows:**
2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake` – optimized binaries.
3. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake` – includes debug symbols.

**For Linux/macOS:**
2. Configure a release build using `cmake -S . -B bin/release -DCMAKE_BUILD_TYPE=Release` – optimized binaries.
3. Configure a debug build using `cmake -S . -B bin/debug -DCMAKE_BUILD_TYPE=Debug` – includes debug symbols.
4. Configure a coverage build using `cmake -S . -B bin/coverage -DOSMMAPMAKER_ENABLE_COVERAGE=ON` – automatically sets a Debug build and enables GCC coverage profiling.
5. Configure a valgrind build using `cmake -S . -B bin/valgrind -DOSMMAPMAKER_ENABLE_VALGRIND=ON` – automatically sets a Debug build with flags suitable for valgrind analysis.

### Build

Build each directory with `cmake --build bin/<type> -j$(nproc)`.

