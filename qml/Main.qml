import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "components"

Window {
    id: root
    visible: true
    width: 1040
    height: 720
    minimumWidth: 900
    minimumHeight: 620
    title: app.appName
    color: "#F7F7F8"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Minecraft 版本"
                font.pixelSize: 16
                font.bold: true
                color: "#1F2328"
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "下载源：自动"
                font.pixelSize: 12
                color: "#6B7280"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 640
                spacing: 10

                LatestCard {
                    Layout.fillWidth: true
                    release: app.latestRelease
                    snapshot: app.latestSnapshot
                }

                ListView {
                    id: versionList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8
                    model: app.versionModel
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    delegate: Item {
                        width: versionList.width
                        height: model.header ? 44 : 40

                        SectionCard {
                            visible: model.header
                            anchors.fill: parent
                            title: model.header ? model.groupName : ""
                            count: model.header ? model.count : 0
                            expanded: model.header ? model.expanded : false
                            onToggled: app.versionModel.toggleGroup(model.group)
                        }

                        VersionRow {
                            visible: !model.header
                            anchors.fill: parent
                            versionId: model.header ? "" : model.versionId
                            displayId: model.header ? "" : model.displayId
                            releaseTime: model.header ? "" : model.releaseTime
                            iconSource: model.header ? "" : model.icon
                            selected: model.header ? false : model.selected
                            onClicked: app.selectedVersion = model.versionId
                        }
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
                color: "#FFFFFF"
                radius: 4
                border.width: 1
                border.color: "#D9DDE3"

                Text {
                    anchors.centerIn: parent
                    text: "安装设置"
                    font.pixelSize: 13
                    color: "#6B7280"
                }
            }
        }
    }
}
