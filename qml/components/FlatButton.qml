import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: control

    property bool primary: true

    implicitHeight: 34
    implicitWidth: 96
    font.pixelSize: 12

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.primary ? "#FFFFFF" : "#1F2328"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 4
        color: {
            if (!control.enabled)
                return "#E5E7EB"
            if (control.primary)
                return control.pressed ? "#1D4ED8" : (control.hovered ? "#3B82F6" : "#2563EB")
            return control.pressed ? "#E5E7EB" : (control.hovered ? "#F3F4F6" : "#FFFFFF")
        }
        border.width: control.primary ? 0 : 1
        border.color: "#D9DDE3"
    }
}
