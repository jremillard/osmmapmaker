cmake_minimum_required(VERSION 3.20)


project(tests LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)

add_executable(hello_test hello_test.cpp)
target_link_libraries(hello_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(hello_test PRIVATE cxx_std_17)

add_test(NAME hello_test COMMAND hello_test)

add_executable(textfieldprocessor_test textfieldprocessor_test.cpp)
target_link_libraries(textfieldprocessor_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(textfieldprocessor_test PRIVATE cxx_std_17)
add_test(NAME textfieldprocessor_test COMMAND textfieldprocessor_test)

add_executable(tileoutput_test tileoutput_test.cpp)
target_link_libraries(tileoutput_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(tileoutput_test PRIVATE cxx_std_17)
add_test(NAME tileoutput_test COMMAND tileoutput_test)

add_executable(project_schema_test
    project_schema_test.cpp
    ../mapmaker/resources.qrc
)
set_target_properties(project_schema_test PROPERTIES AUTORCC ON)
target_link_libraries(project_schema_test PRIVATE Catch2::Catch2WithMain mapmaker Qt5::XmlPatterns)
target_compile_features(project_schema_test PRIVATE cxx_std_17)
target_compile_definitions(project_schema_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME project_schema_test COMMAND project_schema_test)

add_executable(labelpriority_test labelpriority_test.cpp)
target_link_libraries(labelpriority_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(labelpriority_test PRIVATE cxx_std_17)
add_test(NAME labelpriority_test COMMAND labelpriority_test)

add_executable(point_test point_test.cpp)
target_link_libraries(point_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(point_test PRIVATE cxx_std_17)
add_test(NAME point_test COMMAND point_test)

add_executable(line_test line_test.cpp)
target_link_libraries(line_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(line_test PRIVATE cxx_std_17)
add_test(NAME line_test COMMAND line_test)

add_executable(area_test area_test.cpp)
target_link_libraries(area_test PRIVATE Catch2::Catch2WithMain mapmaker Qt5::Xml)
target_compile_features(area_test PRIVATE cxx_std_17)
add_test(NAME area_test COMMAND area_test)

add_executable(label_test label_test.cpp)
target_link_libraries(label_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(label_test PRIVATE cxx_std_17)
add_test(NAME label_test COMMAND label_test)

add_executable(osmdata_test
    osmdata_test.cpp
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(osmdata_test PROPERTIES AUTORCC ON)
target_link_libraries(osmdata_test PRIVATE Catch2::Catch2WithMain mapmaker BZip2::BZip2 ZLIB::ZLIB EXPAT::EXPAT PROJ::proj SQLiteCpp SQLite::SQLite3 GEOS::geos GEOS::geos_c)
target_compile_features(osmdata_test PRIVATE cxx_std_17)
target_compile_definitions(osmdata_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME osmdata_test COMMAND osmdata_test)

add_executable(linebreaking_test linebreaking_test.cpp)
target_link_libraries(linebreaking_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(linebreaking_test PRIVATE cxx_std_17)
add_test(NAME linebreaking_test COMMAND linebreaking_test)

add_executable(project_load_save_test
    project_load_save_test.cpp
    ../mapmaker/resources.qrc
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(project_load_save_test PROPERTIES AUTORCC ON)
target_link_libraries(project_load_save_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::XmlPatterns
    BZip2::BZip2
    ZLIB::ZLIB
    EXPAT::EXPAT
    PROJ::proj
    SQLiteCpp
    SQLite::SQLite3
    GEOS::geos
    GEOS::geos_c
)
target_compile_features(project_load_save_test PRIVATE cxx_std_17)
target_compile_definitions(project_load_save_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME project_load_save_test COMMAND project_load_save_test)
