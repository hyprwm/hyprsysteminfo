pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.hyprland.systeminfo

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

    SystemPalette {
        id: system
        colorGroup: SystemPalette.Active
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
                    visible: SystemInfo.systemLogoName != ""
                    source: "image://systemIcons/" + SystemInfo.systemLogoName
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
                        color: system.windowText
                        text: SystemInfo.systemName
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        color: system.windowText
                        text: SystemInfo.systemUrl
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        color: system.windowText
                        text: SystemInfo.systemKernel
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            Item { visible: hyprlandInfo.visible; Layout.fillWidth: true }
            VSeparator { visible: hyprlandInfo.visible }
            Item { visible: hyprlandInfo.visible; Layout.fillWidth: true }

            RowLayout {
                id: hyprlandInfo
                visible: SystemInfo.hyprlandVersionLong != ""
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
                        color: system.windowText
                        text: "Hyprland"
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        color: system.windowText
                        text: SystemInfo.hyprlandVersion
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Label {
                        color: system.windowText
                        visible: SystemInfo.hyprlandVersion != text
                        text: SystemInfo.hyprlandVersionLong
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
                color: system.windowText
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }

            DetailsLabel { text: "User: " + SystemInfo.user; visible: text != "" }
            DetailsLabel { text: "Model: " + SystemInfo.model; visible: text != "" }
            DetailsLabel { text: "CPU: " + SystemInfo.cpuInfo }

            Repeater {
                model: SystemInfo.gpuInfo

                DetailsLabel {
                    required property string modelData
                    text: "GPU: " + modelData
                }
            }

            DetailsLabel { text: "Memory: " + SystemInfo.ramInfo }
            DetailsLabel { text: "DE: " + SystemInfo.de }
            DetailsLabel { text: "Uptime: " + SystemInfo.uptime }
            DetailsLabel { text: "Displays: " + SystemInfo.screens }
        }

        Item { Layout.fillHeight: true }

        HSeparator {}

        RowLayout {
            visible: SystemInfo.hyprlandVersionLong != ""
            spacing: 6
            Layout.leftMargin: 20
            Layout.rightMargin: 20

            Button {
                text: "Copy Hyprland System Info"
                onClicked: SystemInfo.copySystemInfo();
            }

            Button {
                text: "Copy Hyprland Version"
                onClicked: SystemInfo.copyVersion();
            }
        }
    }
}
