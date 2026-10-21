import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: card

    property string title: ""
    property int count: 0
    property bool expanded: false
    signal toggled()

    height: 44
    color: "#FFFFFF"
    radius: 4
    border.width: 1
    border.color: "#D9DDE3"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        spacing: 8

        Text {
            text: card.title
            font.pixelSize: 13
            font.bold: true
            color: "#1F2328"
        }

        Text {
            text: card.count > 0 ? "(" + card.count + ")" : ""
            font.pixelSize: 12
            color: "#6B7280"
        }

        Item { Layout.fillWidth: true }

        Text {
            text: card.expanded ? "▴" : "▾"
            font.pixelSize: 13
            color: "#6B7280"
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: card.toggled()
    }
}
