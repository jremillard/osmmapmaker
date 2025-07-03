cd vcpkg

rem Install all libraries required by the CMake build files
vcpkg install qt5-base qt5-tools qt5-xmlpatterns ^
    bzip2 zlib expat proj4 sqlite3 sqlitecpp geos libosmium catch2 ^
    --triplet x64-windows

cd ..




