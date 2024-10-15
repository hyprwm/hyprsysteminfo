cmake_minimum_required(VERSION 3.16)

# Get version
file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VER)

project(hsi VERSION ${VER} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Quick QuickControls2)
find_package(PkgConfig REQUIRED)

pkg_check_modules(hyprutils REQUIRED IMPORTED_TARGET hyprutils)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(src)

qt_add_resources(hyprsysteminfo "resource"
    PREFIX "/"
    FILES
        resource/hyprlandlogo.svg
)

include(GNUInstallDirs)
install(TARGETS hyprsysteminfo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES assets/install/hyprsysteminfo.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
