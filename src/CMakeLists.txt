qt_add_executable(hyprsysteminfo
    main.cpp
    util/Utils.cpp
    SystemInfo.cpp
    SystemIconProvider.cpp
)

qt_add_qml_module(hyprsysteminfo
    URI org.hyprland.systeminfo
    VERSION 1.0
    QML_FILES main.qml
)

target_link_libraries(hyprsysteminfo PRIVATE Qt6::Widgets Qt6::QuickControls2 PkgConfig::hyprutils)
