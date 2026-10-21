import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: card

    property var release: ({})
    property var snapshot: ({})

    color: "#FFFFFF"
    radius: 4
    border.width: 1
    border.color: "#D9DDE3"
    height: content.implicitHeight + 28

    Column {
        id: content
        x: 14
        y: 14
        width: parent.width - 28
        spacing: 4

        Text {
            text: "最新版本"
            font.pixelSize: 13
            font.bold: true
            color: "#1F2328"
            height: 22
        }

        Rectangle {
            id: releaseRow
            width: parent.width
            height: 40
            visible: card.release && card.release.id !== undefined && card.release.id !== ""
            color: releaseMouse.containsMouse ? "#F0F5FF" : "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Image {
                    source: card.release.icon !== undefined ? card.release.icon : ""
                    sourceSize.width: 28
                    sourceSize.height: 28
                    width: 28
                    height: 28
                    fillMode: Image.PreserveAspectFit
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        text: card.release.displayId !== undefined ? card.release.displayId : ""
                        font.pixelSize: 13
                        font.bold: true
                        color: "#1F2328"
                    }

                    Text {
                        text: "最新正式版，发布于 " + (card.release.releaseTime !== undefined ? card.release.releaseTime : "")
                        font.pixelSize: 11
                        color: "#6B7280"
                    }
                }
            }

            MouseArea {
                id: releaseMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: app.selectedVersion = card.release.id
            }
        }

        Rectangle {
            id: snapshotRow
            width: parent.width
            height: 40
            visible: card.snapshot && card.snapshot.id !== undefined && card.snapshot.id !== ""
            color: snapshotMouse.containsMouse ? "#F0F5FF" : "transparent"

            RowLayout {
                anchors.fill: parent
                spacing: 10

                Image {
                    source: card.snapshot.icon !== undefined ? card.snapshot.icon : ""
                    sourceSize.width: 28
                    sourceSize.height: 28
                    width: 28
                    height: 28
                    fillMode: Image.PreserveAspectFit
                }

                Column {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        text: card.snapshot.displayId !== undefined ? card.snapshot.displayId : ""
                        font.pixelSize: 13
                        font.bold: true
                        color: "#1F2328"
                    }

                    Text {
                        text: "最新预览版，发布于 " + (card.snapshot.releaseTime !== undefined ? card.snapshot.releaseTime : "")
                        font.pixelSize: 11
                        color: "#6B7280"
                    }
                }
            }

            MouseArea {
                id: snapshotMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: app.selectedVersion = card.snapshot.id
            }
        }
    }
}
