cmake_minimum_required(VERSION 3.20)

project(osmmapmakerapp LANGUAGES CXX)

if(WIN32)
  set(APP_TYPE WIN32)
endif()

set(SRC_FILES
    main.cpp
    mainwindow.cpp
    dataTab.cpp
    outputTab.cpp
    styleTab.cpp
    newtoplevelstyle.cpp
    newprojectdialog.cpp
    outputtypedialog.cpp
    inputtypedialog.cpp
    subLayerTextPage.cpp
    sublayerselectpage.cpp
    selectvalueeditdialog.cpp
    colorpickerdialog.cpp
    mapcanvas.cpp
    boundingboxdialog.cpp)
set(UI_FILES
    mainwindow.ui
    dataTab.ui
    outputTab.ui
    styleTab.ui
    newtoplevelstyle.ui
    newprojectdialog.ui
    outputtypedialog.ui
    inputtypedialog.ui
    subLayerTextPage.ui
    sublayerselectpage.ui
    selectvalueeditdialog.ui
    colorpickerdialog.ui)

add_executable(osmmapmakerapp ${APP_TYPE} ${SRC_FILES} ${UI_FILES} resources.qrc)

set_target_properties(osmmapmakerapp PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_compile_features(osmmapmakerapp PRIVATE cxx_std_17)

target_link_libraries(osmmapmakerapp
    PRIVATE
      mapmaker
      Qt5::Widgets
      BZip2::BZip2
      ZLIB::ZLIB
      EXPAT::EXPAT
      PROJ::proj
  SQLiteCpp
  SQLite::SQLite3
  GEOS::geos
  GEOS::geos_c
)

if(WIN32)
  target_link_libraries(osmmapmakerapp PRIVATE Qt5::WinMain)
endif()

#install(DIRECTORY "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Debug/bin/plugins" DESTINATION  "${CMAKE_BINARY_DIR}/Debug")
