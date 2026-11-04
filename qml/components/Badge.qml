import QtQuick 2.15

// Small label chip used for version channel and freshness, never for buttons.
Rectangle {
    id: badge

    property string label: ""
    property color fill: "#F3F5F8"
    property color ink: "#444C5C"

    implicitHeight: 20
    implicitWidth: text.implicitWidth + 12
    radius: 4
    color: badge.fill

    Text {
        id: text
        anchors.centerIn: parent
        text: badge.label
        color: badge.ink
        font.family: "Microsoft YaHei UI"
        font.pixelSize: 11
        font.weight: Font.DemiBold
        font.letterSpacing: 0.4
    }
}
