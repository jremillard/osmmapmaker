cmake_minimum_required(VERSION 3.10)

project(mapmaker)

set(SRC_FILES project.cpp osmdatadownload.cpp datasource.cpp osmdata.cpp osmdatafile.cpp)

set(CMAKE_AUTOMOC ON)

add_library(mapmaker STATIC ${SRC_FILES} )

target_link_libraries(mapmaker Qt5::Widgets Qt5::Xml)