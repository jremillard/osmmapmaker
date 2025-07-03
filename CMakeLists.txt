cmake_minimum_required(VERSION 3.20)

project(osmmapmaker VERSION 1.0 LANGUAGES CXX)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Xml XmlPatterns)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(SQLiteCpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(PROJ REQUIRED)
find_package(GEOS REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  add_compile_definitions(
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
    _HAS_AUTO_PTR_ETC
  )
endif()

add_subdirectory(mapmaker)
add_subdirectory(osmmapmakerapp)
enable_testing()
add_subdirectory(tests)
