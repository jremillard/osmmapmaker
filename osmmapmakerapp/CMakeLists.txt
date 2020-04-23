cmake_minimum_required(VERSION 3.10)

project(osmmapmakerapp)

cmake_policy(SET CMP0020 NEW)

# Make this a GUI application on Windows
if(WIN32)
  set(CMAKE_WIN32_EXECUTABLE ON)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SRC_FILES  main.cpp mainwindow.cpp dataTab.cpp outputTab.cpp styleTab.cpp)
set(UI_FILES  mainwindow.ui dataTab.ui outputTab.ui styleTab.ui)

add_executable(osmmapmakerapp ${SRC_FILES} ${UI_FILES} resources.qrc)

target_link_libraries(osmmapmakerapp Qt5::Widgets Qt5::WinMain)


