import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// Loader choice for the install rail: one row of channels plus the version of
// that channel. Vanilla hides the version row entirely, so the common case
// costs a single line.
ColumnLayout {
    id: section

    readonly property var kinds: [
        { key: "vanilla", label: "原版" },
        { key: "fabric", label: "Fabric" },
        { key: "quilt", label: "Quilt" },
        { key: "forge", label: "Forge" },
        { key: "neoforge", label: "NeoForge" }
    ]

    spacing: 9

    enabled: !app.installing
    opacity: app.installing ? 0.55 : 1.0
    Behavior on opacity { NumberAnimation { duration: 160 } }

    Text {
        text: "加载器"
        font.family: T.fontUi
        font.pixelSize: 11
        font.weight: Font.DemiBold
        color: T.ink2
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 5

        Repeater {
            model: section.kinds

            delegate: Rectangle {
                id: chip

                readonly property bool current: app.loaderKind === modelData.key

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                radius: 7
                color: chip.current ? T.brandSoft
                                    : (chipMouse.containsMouse ? "#EFF3F9" : T.surfaceSunken)
                border.width: 1
                border.color: chip.current ? "#A9C6F5"
                                           : (chipMouse.containsMouse ? "#D9DFE9" : "transparent")

                Behavior on color { ColorAnimation { duration: 110 } }
                Behavior on border.color { ColorAnimation { duration: 110 } }

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    font.family: T.fontUi
                    font.pixelSize: 10
                    font.weight: chip.current ? Font.DemiBold : Font.Medium
                    color: chip.current ? T.brandInk : T.ink2
                }

                MouseArea {
                    id: chipMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: app.loaderKind = modelData.key
                }
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 7
        visible: app.loaderKind !== "vanilla"

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: "加载器版本"
                font.family: T.fontUi
                font.pixelSize: 11
                color: T.ink3
            }

            Item { Layout.fillWidth: true }

            Text {
                visible: picker.count === 0 && app.loaderLoading
                text: "获取中"
                font.family: T.fontUi
                font.pixelSize: 10
                color: T.ink4
            }

            Text {
                visible: picker.count === 0 && !app.loaderLoading
                text: "该版本暂无可用加载器"
                font.family: T.fontUi
                font.pixelSize: 10
                color: T.ink4
            }
        }

        FlatComboBox {
            id: picker
            Layout.fillWidth: true
            model: app.loaderModel
            textRole: "version"
            placeholder: "暂无可用版本"
            onActivated: app.loaderVersion = picker.textAt(index)
        }
    }
}
