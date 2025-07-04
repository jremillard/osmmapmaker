cmake_minimum_required(VERSION 3.20)


project(mapmaker LANGUAGES CXX)

set(SRC_FILES
    project.cpp
    osmdataextractdownload.cpp
    osmdatadirectdownload.cpp
    osmdataoverpass.cpp
    renderdatabase.cpp
    datasource.cpp
    osmdata.cpp
    osmdatafile.cpp
    tilecache.cpp
    stylelayer.cpp
    textfield.cpp
    output.cpp
    renderqt.cpp
    linebreaking.cpp
)
add_library(mapmaker STATIC ${SRC_FILES} resources.qrc)

target_include_directories(mapmaker
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(mapmaker PROPERTIES AUTOMOC ON AUTORCC ON)

target_compile_features(mapmaker PUBLIC cxx_std_17)

target_link_libraries(mapmaker
    PUBLIC
        Qt5::Widgets
        Qt5::Xml
        Qt5::XmlPatterns
        Qt5::Network
)

