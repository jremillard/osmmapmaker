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

add_executable(style_selector_test style_selector_test.cpp)
target_link_libraries(style_selector_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(style_selector_test PRIVATE cxx_std_17)
add_test(NAME style_selector_test COMMAND style_selector_test)
