cmake_minimum_required(VERSION 3.10)

project(osmmapmaker)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Xml)
find_package(BZip2)
find_package(ZLib)
find_package(EXPAT)
find_package(SQLiteCpp)
find_package(sqlite3)
find_package(PROJ4)

add_definitions(-DNOMINMAX)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_WIN32_WINNT=0x0601)
add_definitions(-DWIN32_LEAN_AND_MEAN)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /std:c++17")

include_directories("${PROJECT_SOURCE_DIR}/mapmaker")

add_subdirectory(mapmaker)
add_subdirectory(osmmapmakerapp)
