cmake_minimum_required(VERSION 3.10)

project(mapmaker LANGUAGES CXX)

set(SRC_FILES project.cpp osmdataextractdownload.cpp osmdatadirectdownload.cpp datasource.cpp osmdata.cpp osmdatafile.cpp tilecache.cpp stylelayer.cpp textfield.cpp output.cpp renderqt.cpp linebreaking.cpp)

set(CMAKE_AUTOMOC ON)

add_library(mapmaker STATIC ${SRC_FILES} )

target_link_libraries(mapmaker Qt5::Widgets Qt5::Xml)