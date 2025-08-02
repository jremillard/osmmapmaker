cmake_minimum_required(VERSION 3.20)


project(tests LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)

add_executable(hello_test hello_test.cpp)
target_link_libraries(hello_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(hello_test PRIVATE cxx_std_17)

add_test(NAME hello_test COMMAND hello_test)

add_executable(maputils_test maputils_test.cpp)
target_link_libraries(maputils_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(maputils_test PRIVATE cxx_std_17)
add_test(NAME maputils_test COMMAND maputils_test)

add_executable(textfieldprocessor_test textfieldprocessor_test.cpp)
target_link_libraries(textfieldprocessor_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(textfieldprocessor_test PRIVATE cxx_std_17)
add_test(NAME textfieldprocessor_test COMMAND textfieldprocessor_test)

add_executable(demdata_test demdata_test.cpp)
if(WIN32)
  target_link_libraries(demdata_test PRIVATE Catch2::Catch2WithMain mapmaker SQLiteCpp unofficial::sqlite3::sqlite3)
else()
  target_link_libraries(demdata_test PRIVATE Catch2::Catch2WithMain mapmaker SQLiteCpp SQLite::SQLite3)
endif()
target_compile_features(demdata_test PRIVATE cxx_std_17)
target_compile_definitions(demdata_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME demdata_test COMMAND demdata_test)

add_executable(tileoutput_test tileoutput_test.cpp)
target_link_libraries(tileoutput_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(tileoutput_test PRIVATE cxx_std_17)
add_test(NAME tileoutput_test COMMAND tileoutput_test)

add_executable(imageoutput_test imageoutput_test.cpp)
target_link_libraries(imageoutput_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(imageoutput_test PRIVATE cxx_std_17)
add_test(NAME imageoutput_test COMMAND imageoutput_test)

add_executable(batchtileoutput_test batchtileoutput_test.cpp)
target_link_libraries(batchtileoutput_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    BZip2::BZip2
    ZLIB::ZLIB
    expat::expat
    PROJ::proj
    SQLiteCpp
    GEOS::geos
    GEOS::geos_c
)
if(WIN32)
    target_link_libraries(batchtileoutput_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(batchtileoutput_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(batchtileoutput_test PRIVATE cxx_std_17)
add_test(NAME batchtileoutput_test COMMAND batchtileoutput_test)

add_executable(project_schema_test
    project_schema_test.cpp
)
target_link_libraries(project_schema_test PRIVATE Catch2::Catch2WithMain mapmaker Qt5::XmlPatterns)
target_compile_features(project_schema_test PRIVATE cxx_std_17)
target_compile_definitions(project_schema_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME project_schema_test COMMAND project_schema_test)

add_executable(labelpriority_test labelpriority_test.cpp)
target_link_libraries(labelpriority_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(labelpriority_test PRIVATE cxx_std_17)
add_test(NAME labelpriority_test COMMAND labelpriority_test)

add_executable(stylelayer_test stylelayer_test.cpp)
target_link_libraries(stylelayer_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(stylelayer_test PRIVATE cxx_std_17)
add_test(NAME stylelayer_test COMMAND stylelayer_test)

add_executable(style_selector_test style_selector_test.cpp)
target_link_libraries(style_selector_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(style_selector_test PRIVATE cxx_std_17)
add_test(NAME style_selector_test COMMAND style_selector_test)

add_executable(sublayer_test sublayer_test.cpp)
target_link_libraries(sublayer_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(sublayer_test PRIVATE cxx_std_17)
add_test(NAME sublayer_test COMMAND sublayer_test)

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
target_link_libraries(osmdata_test PRIVATE Catch2::Catch2WithMain mapmaker BZip2::BZip2 ZLIB::ZLIB expat::expat PROJ::proj SQLiteCpp GEOS::geos GEOS::geos_c)
if(WIN32)
    target_link_libraries(osmdata_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(osmdata_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(osmdata_test PRIVATE cxx_std_17)
target_compile_definitions(osmdata_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME osmdata_test COMMAND osmdata_test)

add_executable(linebreaking_test linebreaking_test.cpp)
target_link_libraries(linebreaking_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(linebreaking_test PRIVATE cxx_std_17)
add_test(NAME linebreaking_test COMMAND linebreaking_test)

add_executable(project_load_save_test
    project_load_save_test.cpp
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(project_load_save_test PROPERTIES AUTORCC ON)
target_link_libraries(project_load_save_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::XmlPatterns
    BZip2::BZip2
    ZLIB::ZLIB
    expat::expat
    PROJ::proj
    SQLiteCpp
    GEOS::geos
    GEOS::geos_c
)
if(WIN32)
    target_link_libraries(project_load_save_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(project_load_save_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(project_load_save_test PRIVATE cxx_std_17)
target_compile_definitions(project_load_save_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME project_load_save_test COMMAND project_load_save_test)

add_executable(new_project_test
    new_project_test.cpp
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(new_project_test PROPERTIES AUTORCC ON)
target_link_libraries(new_project_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::XmlPatterns
    BZip2::BZip2
    ZLIB::ZLIB
    expat::expat
    PROJ::proj
    SQLiteCpp
    GEOS::geos
    GEOS::geos_c
)
if(WIN32)
    target_link_libraries(new_project_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(new_project_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(new_project_test PRIVATE cxx_std_17)
target_compile_definitions(new_project_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME new_project_test COMMAND new_project_test)

add_executable(projecttemplate_test
    projecttemplate_test.cpp
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(projecttemplate_test PROPERTIES AUTORCC ON)
target_link_libraries(projecttemplate_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::XmlPatterns
    BZip2::BZip2
    ZLIB::ZLIB
    expat::expat
    PROJ::proj
    SQLiteCpp
    GEOS::geos
    GEOS::geos_c
)
if(WIN32)
    target_link_libraries(projecttemplate_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(projecttemplate_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(projecttemplate_test PRIVATE cxx_std_17)
target_compile_definitions(projecttemplate_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME projecttemplate_test COMMAND projecttemplate_test)

add_executable(overpass_test
    overpass_test.cpp
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(overpass_test PROPERTIES AUTORCC ON)
target_link_libraries(overpass_test PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::Network
    BZip2::BZip2
    ZLIB::ZLIB
    expat::expat
    PROJ::proj
    SQLiteCpp
    GEOS::geos
    GEOS::geos_c
)
if(WIN32)
    target_link_libraries(overpass_test PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(overpass_test PRIVATE SQLite::SQLite3)
endif()
target_compile_features(overpass_test PRIVATE cxx_std_17)
target_compile_definitions(overpass_test PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME overpass_test COMMAND overpass_test)

add_executable(applicationpreferences_test applicationpreferences_test.cpp)
target_link_libraries(applicationpreferences_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(applicationpreferences_test PRIVATE cxx_std_17)
add_test(NAME applicationpreferences_test COMMAND applicationpreferences_test)
