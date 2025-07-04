@echo off
REM Navigate into the vcpkg directory
cd vcpkg

REM Fetch tags (once) to ensure the 2025-06-02 tag is available
git fetch --tags

REM Pin vcpkg to the latest tagged release
git checkout 2025-06-02

REM Install all libraries required by the CMake build files
vcpkg install qt5-base qt5-tools qt5-xmlpatterns ^
    bzip2 zlib expat proj4 sqlite3 sqlitecpp geos libosmium catch2 ^
    --triplet x64-windows

cd ..