cmake_minimum_required(VERSION 3.10)

project(mapmaker)

set(SRC_FILES project.cpp)

add_library(mapmaker STATIC ${SRC_FILES} )