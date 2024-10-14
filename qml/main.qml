pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    FontMetrics { id: fontMetrics }

    property var firstPanelHeight: fontMetrics.height * 7

    minimumWidth: Math.max(fontMetrics.height * 50, mainLayout.Layout.minimumWidth) + mainLayout.anchors.margins * 2
    minimumHeight: Math.max(fontMetrics.height * 30, mainLayout.Layout.minimumHeight) + mainLayout.anchors.margins * 2
    maximumWidth: minimumWidth
    maximumHeight: minimumHeight
    visible: true

    component Separator: Rectangle {
        color: Qt.darker(window.palette.text, 1.5)
    }

    component VSeparator: Separator {
        implicitWidth: 1
        Layout.fillHeight: true
        Layout.topMargin: fontMetrics.height
        Layout.bottomMargin: fontMetrics.height
    }

    component HSeparator: Separator {
        implicitHeight: 1
        Layout.fillWidth: true
        Layout.leftMargin: fontMetrics.height * 8
        Layout.rightMargin: fontMetrics.height * 8
    }

    ColumnLayout {
        id: mainLayout
        spacing: fontMetrics.height

        anchors {
            fill: parent
            margins: 4
        }

        RowLayout {
            // First panel hyprland and distro info
            Layout.preferredHeight: firstPanelHeight
            Layout.maximumHeight: firstPanelHeight
            Layout.topMargin: fontMetrics.height
            spacing: fontMetrics.height

            Item { Layout.fillWidth: true }

            RowLayout {
                id: distroLogoName
                spacing: fontMetrics.height

                Image {
                    visible: hsi.hasSystemLogoName()
                    source: "image://systemIcons/" + hsi.getSystemLogoName()
                    sourceSize.width: firstPanelHeight
                    sourceSize.height: firstPanelHeight
                    Layout.preferredWidth: firstPanelHeight
                    Layout.preferredHeight: firstPanelHeight
                    Layout.alignment: Qt.AlignCenter
                    smooth: true
                }

                ColumnLayout {
                    id: distroText

                    Layout.preferredWidth: hyprlandInfo.visible ? Math.max(Layout.minimumWidth, hyprlandText.Layout.minimumWidth) : Layout.minimumWidth
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter

                    Label {
                        text: hsi.getSystemName()
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: hsi.getSystemURL()
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: hsi.getSystemKernel()
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Item { visible: hyprlandInfo.visible; Layout.fillWidth: true }
            VSeparator { visible: hyprlandInfo.visible }
            Item { visible: hyprlandInfo.visible; Layout.fillWidth: true }

            RowLayout {
                id: hyprlandInfo
                visible: hsi.hasHyprland()
                spacing: fontMetrics.height

                Image {
                    source: "qrc:/resource/hyprlandlogo.svg"
                    sourceSize.width: firstPanelHeight
                    sourceSize.height: firstPanelHeight
                    Layout.preferredWidth: firstPanelHeight
                    Layout.preferredHeight: firstPanelHeight
                    Layout.alignment: Qt.AlignCenter
                    smooth: true
                }

                ColumnLayout {
                    id: hyprlandText
                    Layout.preferredWidth: Math.max(Layout.minimumWidth, distroText.Layout.minimumWidth)
                    spacing: 2
                    Layout.alignment: Qt.AlignVCenter

                    Label {
                        text: "Hyprland"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        text: hsi.getHyprlandVersion()
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        visible: hsi.getHyprlandVersion() != hsi.getHyprlandVersionLong()
                        text: hsi.getHyprlandVersionLong()
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Item { Layout.fillWidth: true }
        }

        HSeparator {}

        ColumnLayout {
            spacing: 6
            Layout.leftMargin: fontMetrics.height * 4
            Layout.rightMargin: fontMetrics.height * 4

            component DetailsLabel: Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }

            DetailsLabel { text: "User: " + hsi.getUserAt(); visible: hsi.getUserAt() != "" }
            DetailsLabel { text: "Model: " + hsi.getModel(); visible: hsi.getModel() != "" }
            DetailsLabel { text: "CPU: " + hsi.getCPUInfo() }

            Repeater {
                model: hsi.getGPUInfoCount()

                DetailsLabel {
                    required property int index
                    text: "GPU: " + hsi.getGPUInfo(index)
                }
            }

            DetailsLabel { text: "Memory: " + hsi.getRAMInfo() }
            DetailsLabel { text: "DE: " + hsi.getDE() }
            DetailsLabel { text: "Uptime: " + hsi.getUptime() }
            DetailsLabel { text: "Displays: " + hsi.getDisplays() }
        }

        Item { Layout.fillHeight: true }

        HSeparator {}

        RowLayout {
            visible: hsi.hasHyprland()
            spacing: 6
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            Button {
                text: "Copy Hyprland System Info"
                onClicked: hsi.copySystemInfo();
            }

            Button {
                text: "Copy Hyprland Version"
                onClicked: hsi.copyVersion();
            }
        }
    }
}
