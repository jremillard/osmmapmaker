cmake_minimum_required(VERSION 3.10)

project(osmmapmaker VERSION 1.0 LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Xml)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PROJ4 REQUIRED)
find_package(GEOS REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_HAS_AUTO_PTR_ETC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include_directories("${PROJECT_SOURCE_DIR}/mapmaker")

add_subdirectory(mapmaker)
add_subdirectory(osmmapmakerapp)
add_subdirectory(tests)
