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
    colorpickerdialog.cpp)
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
      expat::expat
      PROJ::proj
  SQLiteCpp
  GEOS::geos
  GEOS::geos_c
)

if(WIN32)
  target_link_libraries(osmmapmakerapp PRIVATE Qt5::WinMain unofficial::sqlite3::sqlite3)
else()
  target_link_libraries(osmmapmakerapp PRIVATE SQLite::SQLite3)
endif()

if(WIN32)
    find_file(PROJ_DB_PATH proj.db HINTS ENV VCPKG_INSTALLED_DIR PATH_SUFFIXES "share/proj" "x64-windows/share/proj" "x86-windows/share/proj")
    if(PROJ_DB_PATH)
        add_custom_command(TARGET osmmapmakerapp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                "${PROJ_DB_PATH}"
                "$<TARGET_FILE_DIR:osmmapmakerapp>"
            COMMENT "Copying proj.db to executable directory")
    else()
        message(WARNING "proj.db not found. PROJ may not function correctly.")
    endif()
endif()

#install(DIRECTORY "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Debug/bin/plugins" DESTINATION  "${CMAKE_BINARY_DIR}/Debug")
