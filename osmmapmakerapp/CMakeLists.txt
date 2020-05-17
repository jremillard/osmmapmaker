cmake_minimum_required(VERSION 3.10)

project(osmmapmakerapp)

cmake_policy(SET CMP0020 NEW)

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SRC_FILES main.cpp mainwindow.cpp dataTab.cpp outputTab.cpp styleTab.cpp newtoplevelstyle.cpp)
set(UI_FILES mainwindow.ui dataTab.ui outputTab.ui styleTab.ui newtoplevelstyle.ui)

find_library(mapnik NAMES mapnik)

add_executable(osmmapmakerapp ${SRC_FILES} ${UI_FILES} resources.qrc)

target_link_libraries(osmmapmakerapp mapmaker Qt5::Widgets Qt5::WinMain BZip2::BZip2 ZLIB::ZLIB EXPAT::EXPAT ${PROJ4_LIBRARIES} SQLiteCpp sqlite3 ${mapnik} )

#install(DIRECTORY "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Debug/bin/plugins" DESTINATION  "${CMAKE_BINARY_DIR}/Debug")
