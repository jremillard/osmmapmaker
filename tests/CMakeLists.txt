# For instructions on running the tests, see:
# - README.md: For general build setup
# - AGENTS.md: For specific test commands
# Run tests with:
# Windows: ctest --test-dir bin/release -C Release (or --test-dir bin/debug -C Debug)
# Linux/macOS: ctest --test-dir bin/release (or bin/debug, bin/coverage, bin/valgrind)
# Add --output-on-failure to see detailed test results

cmake_minimum_required(VERSION 3.20)

project(tests LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)

set(test_sources
    applicationpreferences_test.cpp
    area_test.cpp
    batchtileoutput_test.cpp
    demdata_test.cpp
    hello_test.cpp
    imageoutput_test.cpp
    label_test.cpp
    labelpriority_test.cpp
    line_test.cpp
    linebreaking_test.cpp
    maputils_test.cpp
    new_project_test.cpp
    osmdata_test.cpp
    overpass_test.cpp
    point_test.cpp
    project_load_save_test.cpp
    project_schema_test.cpp
    projecttemplate_test.cpp
    style_selector_test.cpp
    stylelayer_test.cpp
    sublayer_test.cpp
    textfieldprocessor_test.cpp
    tileoutput_test.cpp
)

add_executable(osmmapmaker_tests
    ${test_sources}
    ../osmmapmakerapp/resources.qrc
)
set_target_properties(osmmapmaker_tests PROPERTIES AUTORCC ON)

# link against all libraries required by individual test files
 target_link_libraries(osmmapmaker_tests PRIVATE
    Catch2::Catch2WithMain
    mapmaker
    Qt5::XmlPatterns
    Qt5::Xml
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
    target_link_libraries(osmmapmaker_tests PRIVATE unofficial::sqlite3::sqlite3)
else()
    target_link_libraries(osmmapmaker_tests PRIVATE SQLite::SQLite3)
endif()

target_compile_features(osmmapmaker_tests PRIVATE cxx_std_17)
target_compile_definitions(osmmapmaker_tests PRIVATE SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_test(NAME osmmapmaker_tests COMMAND osmmapmaker_tests)

if(WIN32)
    find_file(PROJ_DB_PATH proj.db HINTS ENV VCPKG_INSTALLED_DIR PATH_SUFFIXES "share/proj" "x64-windows/share/proj" "x86-windows/share/proj")
    if(PROJ_DB_PATH)
        add_custom_command(TARGET osmmapmaker_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${PROJ_DB_PATH}"
                "$<TARGET_FILE_DIR:osmmapmaker_tests>"
            COMMENT "Copying proj.db to executable directory")
    else()
        message(WARNING "proj.db not found. PROJ may not function correctly.")
    endif()
endif()
