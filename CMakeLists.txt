cmake_minimum_required(VERSION 3.10)

project(osmmapmaker)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core)

include_directories("${PROJECT_SOURCE_DIR}/mapmaker")

add_subdirectory(mapmaker)
add_subdirectory(osmmapmakerapp)

