import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// Group header for the version list. Not a card: a section head that sits
// flush left, names the channel, and rules off the full width of the column.
Item {
    id: header

    property string label: ""
    property int count: 0
    property bool expanded: false

    signal toggled()

    implicitHeight: 40

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        spacing: 8

        Caret {
            Layout.alignment: Qt.AlignVCenter
            expanded: header.expanded
            stroke: mouse.containsMouse ? T.brand : T.ink3
            Behavior on stroke { ColorAnimation { duration: 120 } }
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: header.label
            font.family: T.fontUi
            font.pixelSize: 12
            font.weight: Font.DemiBold
            font.letterSpacing: 0.4
            color: mouse.containsMouse ? T.ink : T.ink2
            Behavior on color { ColorAnimation { duration: 120 } }
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: header.count
            font.family: T.fontUi
            font.pixelSize: 11
            color: T.ink4
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.leftMargin: 4
            color: T.line
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: header.toggled()
    }
}
