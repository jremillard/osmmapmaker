cmake_minimum_required(VERSION 3.10)

project(tests LANGUAGES CXX)

enable_testing()

find_package(Catch2 3 REQUIRED)

add_executable(hello_test hello_test.cpp)
target_link_libraries(hello_test PRIVATE Catch2::Catch2WithMain mapmaker)

add_test(NAME hello_test COMMAND hello_test)
