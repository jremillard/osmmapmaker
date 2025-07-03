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

add_executable(labelpriority_test labelpriority_test.cpp)
target_link_libraries(labelpriority_test PRIVATE Catch2::Catch2WithMain mapmaker)
target_compile_features(labelpriority_test PRIVATE cxx_std_17)
add_test(NAME labelpriority_test COMMAND labelpriority_test)
