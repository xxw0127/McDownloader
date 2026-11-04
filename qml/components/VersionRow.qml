import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// One selectable version. The icon sits in an inventory-style slot: sunken
// ground, crisp nearest-neighbour scaling, and a border that lights up only
// when the row is chosen.
Item {
    id: row

    property string versionId: ""
    property string displayId: ""
    property string releaseTime: ""
    property string iconSource: ""
    property bool selected: false

    signal clicked()

    implicitHeight: 56

    Rectangle {
        id: plate
        anchors.fill: parent
        radius: 9
        color: row.selected ? T.brandSoft : (mouse.containsMouse ? T.surfaceHover : "transparent")
        Behavior on color { ColorAnimation { duration: 120 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 14
        spacing: 12

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 38
            implicitHeight: 38
            radius: 9
            color: row.selected ? "#FFFFFF" : T.surfaceSunken
            border.width: 1
            border.color: row.selected ? "#BBD3FB" : T.lineSoft
            Behavior on color { ColorAnimation { duration: 120 } }

            Image {
                anchors.centerIn: parent
                source: row.iconSource
                sourceSize.width: 32
                sourceSize.height: 32
                width: 32
                height: 32
                fillMode: Image.PreserveAspectFit
                smooth: false
                visible: row.iconSource !== ""
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 3

            Text {
                text: row.displayId
                font.family: T.fontUi
                font.pixelSize: 13
                font.weight: Font.DemiBold
                color: row.selected ? T.brandInk : T.ink
                elide: Text.ElideRight
                width: parent.width
            }
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: row.releaseTime
            font.family: T.fontUi
            font.pixelSize: 11
            color: row.selected ? "#4C6B9E" : T.ink4
            Behavior on color { ColorAnimation { duration: 120 } }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: row.clicked()
    }
}
