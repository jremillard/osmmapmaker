cmake_minimum_required(VERSION 3.10)

project(tests LANGUAGES CXX)

find_package(Qt5Test REQUIRED)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

#enable_testing(true)
#add_test(NAME textsplittest COMMAND textsplittest)

add_executable(textsplittest textsplittest.cpp)
target_link_libraries(textsplittest PRIVATE mapmaker Qt5::Test BZip2::BZip2 ZLIB::ZLIB EXPAT::EXPAT ${PROJ4_LIBRARIES} SQLiteCpp sqlite3 GEOS::geos GEOS::geos_c)

add_executable(projecttest projecttest.cpp)
target_link_libraries(projecttest  PRIVATE mapmaker Qt5::Test BZip2::BZip2 ZLIB::ZLIB EXPAT::EXPAT ${PROJ4_LIBRARIES} SQLiteCpp sqlite3 GEOS::geos GEOS::geos_c)



