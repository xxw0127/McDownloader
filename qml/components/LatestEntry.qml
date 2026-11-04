import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// One of the two quick-pick targets in the header card.
Item {
    id: entry

    property string versionId: ""
    property string displayId: ""
    property string releaseTime: ""
    property string iconSource: ""
    property string channel: ""
    property color channelFill: T.surfaceSunken
    property color channelInk: T.ink2

    signal clicked()

    implicitHeight: 92

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: mouse.containsMouse ? T.surfaceHover : "transparent"
        Behavior on color { ColorAnimation { duration: 130 } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 46
            implicitHeight: 46
            radius: 10
            color: T.surfaceSunken
            border.width: 1
            border.color: mouse.containsMouse ? "#BBD3FB" : T.lineSoft
            Behavior on border.color { ColorAnimation { duration: 130 } }

            Image {
                anchors.centerIn: parent
                source: entry.iconSource
                sourceSize.width: 40
                sourceSize.height: 40
                width: 40
                height: 40
                fillMode: Image.PreserveAspectFit
                smooth: false
                visible: entry.iconSource !== ""
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 5

            Text {
                width: parent.width
                text: entry.displayId
                font.family: T.fontUi
                font.pixelSize: 14
                font.weight: Font.Bold
                color: T.ink
                elide: Text.ElideRight
            }

            Row {
                spacing: 6

                Text {
                    text: entry.releaseTime
                    font.family: T.fontUi
                    font.pixelSize: 11
                    color: T.ink4
                    anchors.verticalCenter: parent.verticalCenter
                }

                Badge {
                    label: entry.channel
                    fill: entry.channelFill
                    ink: entry.channelInk
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: entry.clicked()
    }
}
