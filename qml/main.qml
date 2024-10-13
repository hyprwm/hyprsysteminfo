import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    property var _width: 620
    property var _height: 400
    property var _first_panel_height: 120

    minimumWidth: _width
    maximumWidth: _width
    width: _width
    minimumHeight: _height
    maximumHeight: _height
    height: _height
    visible: true

    SystemPalette {
        id: system

        colorGroup: SystemPalette.Active
    }

    ColumnLayout {
        id: mainLayout

        anchors.margins: 4

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        RowLayout {
            // First panel hyprland and distro info
            Layout.fillWidth: true
            Layout.preferredHeight: _first_panel_height
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                id: distroLogoName

                Layout.preferredWidth: _width / 2 - 40
                spacing: 10

                Image {
                    visible: hsi.hasSystemLogoName()
                    source: "image://systemIcons/" + hsi.getSystemLogoName()
                    sourceSize.width: _first_panel_height - 10
                    sourceSize.height: _first_panel_height - 10
                    Layout.preferredWidth: _first_panel_height - 10
                    Layout.preferredHeight: _first_panel_height - 10
                    Layout.alignment: Qt.AlignCenter
                    smooth: true
                }

                ColumnLayout {
                    spacing: 2

                    Text {
                        text: hsi.getSystemName()
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                    Text {
                        text: hsi.getSystemURL()
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                    Text {
                        text: hsi.getSystemKernel()
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                }

            }

            Rectangle {
                visible: hsi.hasHyprland()
                color: Qt.darker(system.text, 1.5)
                Layout.preferredWidth: 1
                Layout.preferredHeight: _first_panel_height - 40
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.alignment: Qt.AlignVCenter
            }

            RowLayout {
                id: hyprlandInfo

                visible: hsi.hasHyprland()
                Layout.preferredWidth: _width / 2 - 40
                spacing: 10

                Image {
                    source: "qrc:/resource/hyprlandlogo.svg"
                    sourceSize.width: _first_panel_height - 10
                    sourceSize.height: _first_panel_height - 10
                    Layout.preferredWidth: _first_panel_height - 10
                    Layout.preferredHeight: _first_panel_height - 10
                    Layout.alignment: Qt.AlignCenter
                    smooth: true
                }

                ColumnLayout {
                    spacing: 2

                    Text {
                        text: "Hyprland"
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                    Text {
                        text: hsi.getHyprlandVersion()
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                    Text {
                        text: hsi.getHyprlandVersionLong()
                        Layout.alignment: Qt.AlignHCenter
                        color: system.windowText
                    }

                }

            }

        }

        Rectangle {
            color: Qt.darker(system.text, 1.5)
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            Layout.leftMargin: 180
            Layout.rightMargin: 180
            Layout.topMargin: 10
            Layout.bottomMargin: 10
        }

        ColumnLayout {
            spacing: 6
            Layout.leftMargin: 60
            Layout.rightMargin: 60

            Text {
                text: "CPU: " + hsi.getCPUInfo()
                Layout.maximumWidth: _width - 120
                color: system.windowText
                elide: Text.ElideRight
                textFormat: Text.PlainText
                wrapMode: Text.NoWrap
            }

            Repeater {
                model: hsi.getGPUInfoCount()

                Text {
                    required property int index

                    text: "GPU: " + hsi.getGPUInfo(index)
                    Layout.maximumWidth: _width - 120
                    color: system.windowText
                    elide: Text.ElideRight
                    textFormat: Text.PlainText
                    wrapMode: Text.NoWrap
                }

            }

            Text {
                text: "Memory: " + hsi.getRAMInfo()
                Layout.maximumWidth: _width - 120
                color: system.windowText
                elide: Text.ElideRight
                textFormat: Text.PlainText
                wrapMode: Text.NoWrap
            }

        }

        Rectangle {
            color: "transparent"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Rectangle {
            visible: hsi.hasHyprland()
            color: Qt.darker(system.text, 1.5)
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            Layout.leftMargin: 180
            Layout.rightMargin: 180
            Layout.topMargin: 10
            Layout.bottomMargin: 10
        }

        RowLayout {
            visible: hsi.hasHyprland()
            spacing: 6
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            Button {
                text: "Copy Hyprland System Info"
                onClicked: (e) => {
                    hsi.copySystemInfo();
                }
            }

            Button {
                text: "Copy Hyprland Version"
                onClicked: (e) => {
                    hsi.copyVersion();
                }
            }

        }

    }

}
