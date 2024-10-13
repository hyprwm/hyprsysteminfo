cmake_minimum_required(VERSION 3.16)

# Get version
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VER)

project(hsi VERSION ${VER} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Quick QuickControls2)
find_package(PkgConfig REQUIRED)
set(CMAKE_CXX_STANDARD 23)

pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  hyprutils)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(hyprsysteminfo
    src/main.cpp src/util/Utils.cpp src/util/Utils.hpp
)

qt_add_qml_module(hyprsysteminfo
    URI hsi
    VERSION 1.0
    QML_FILES
        qml/main.qml
    SOURCES
        qmlSources/SystemInternals.hpp qmlSources/SystemInternals.cpp qmlSources/SystemIconProvider.hpp
)

qt_add_resources(hyprsysteminfo "resource"
    PREFIX "/"
    FILES
        resource/hyprlandlogo.svg
        resource/hyprlandlogo.png
)

target_link_libraries(hyprsysteminfo
    PRIVATE Qt6::Widgets Qt6::Quick Qt6::Gui Qt6::QuickControls2 PkgConfig::deps
)

include(GNUInstallDirs)
install(TARGETS hyprsysteminfo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/install/hyprsysteminfo.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
