# Build instructions can be found in readme.md and agents.md.

cmake_minimum_required(VERSION 3.20)

project(osmmapmaker VERSION 1.0 LANGUAGES CXX)

option(OSMMAPMAKER_ENABLE_COVERAGE "Enable GCC coverage flags" OFF)
option(OSMMAPMAKER_ENABLE_VALGRIND "Enable flags suitable for valgrind" OFF)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Xml XmlPatterns Network)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
if(WIN32)
  find_package(unofficial-sqlite3 CONFIG REQUIRED)
else()
  find_package(SQLite3 REQUIRED)
endif()
find_package(PROJ CONFIG REQUIRED)
find_package(GEOS CONFIG REQUIRED)
find_package(Catch2 3 CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(OSMMAPMAKER_ENABLE_COVERAGE)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  endif()
  foreach(VAR CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    string(APPEND ${VAR} " -O0 -g --coverage")
  endforeach()
  string(APPEND CMAKE_EXE_LINKER_FLAGS " --coverage")
elseif(OSMMAPMAKER_ENABLE_VALGRIND)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
  endif()
  foreach(VAR CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    string(APPEND ${VAR} " -O0 -g")
  endforeach()
endif()

if(WIN32)
  add_compile_definitions(
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    _WIN32_WINNT=0x0601
    WIN32_LEAN_AND_MEAN
    _HAS_AUTO_PTR_ETC
    _USE_MATH_DEFINES
  )
endif()

add_subdirectory(mapmaker)
add_subdirectory(osmmapmakerapp)
enable_testing()
add_subdirectory(tests)
