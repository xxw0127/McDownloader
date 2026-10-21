import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: row

    property string versionId: ""
    property string displayId: ""
    property string releaseTime: ""
    property string iconSource: ""
    property bool selected: false
    signal clicked()

    height: 40
    color: selected ? "#E8F1FF" : "#FFFFFF"

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: row.selected ? "#2563EB" : "transparent"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        spacing: 10

        Image {
            source: row.iconSource
            sourceSize.width: 28
            sourceSize.height: 28
            width: 28
            height: 28
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: row.displayId
            font.pixelSize: 13
            font.bold: true
            color: "#1F2328"
        }

        Item { Layout.fillWidth: true }

        Text {
            text: row.releaseTime
            font.pixelSize: 12
            color: "#6B7280"
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: row.clicked()
    }
}
