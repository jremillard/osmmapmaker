cd vcpkg

#vcpkg remove harfbuzz --triplet x64-windows
#vcpkg install harfbuzz[icu] --triplet x64-windows

vcpkg install mapnik[cairo,sqlite3] --triplet x64-windows

vcpkg install qt5-tools qt5-imageformats qt5-location --triplet x64-windows

cd ..




