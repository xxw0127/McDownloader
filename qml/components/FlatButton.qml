import QtQuick 2.15
import QtQuick.Controls 2.15

// Single button primitive for the whole app.
// variant: "primary" (the commit action), "secondary" (browse / cancel), "ghost"
Button {
    id: control

    property string variant: "primary"
    property int radius: 8
    property int textSize: 13

    implicitHeight: 36
    implicitWidth: 92
    hoverEnabled: true
    font.family: "Microsoft YaHei UI"

    readonly property bool isPrimary: control.variant === "primary"
    readonly property bool isGhost: control.variant === "ghost"

    readonly property color _top: {
        if (control.isPrimary) {
            if (!control.enabled)
                return "#E3E7EE"
            if (control.pressed)
                return "#1E52AC"
            if (control.hovered)
                return "#4B87F1"
            return "#3A78E6"
        }
        if (control.isGhost)
            return control.hovered ? "#F1F4F9" : "#00000000"
        if (!control.enabled)
            return "#F4F6F9"
        if (control.pressed)
            return "#EDF0F6"
        if (control.hovered)
            return "#F6F8FC"
        return "#FFFFFF"
    }

    readonly property color _bottom: {
        if (control.isPrimary) {
            if (!control.enabled)
                return "#E3E7EE"
            if (control.pressed)
                return "#18438F"
            if (control.hovered)
                return "#3069D8"
            return "#2663CE"
        }
        return control._top
    }

    contentItem: Text {
        text: control.text
        font.family: "Microsoft YaHei UI"
        font.pixelSize: control.textSize
        font.weight: control.isPrimary ? Font.DemiBold : Font.Medium
        color: {
            if (!control.enabled)
                return "#6F7788"
            if (control.isPrimary)
                return "#FFFFFF"
            if (control.isGhost)
                return "#444C5C"
            return "#2C3444"
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: control.radius
        border.width: control.isPrimary ? 0 : 1
        border.color: {
            if (!control.enabled)
                return "#E2E6ED"
            return control.hovered ? "#C3CBDA" : "#DBE0E9"
        }
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: control._top
                Behavior on color { ColorAnimation { duration: 120 } }
            }
            GradientStop {
                position: 1.0
                color: control._bottom
                Behavior on color { ColorAnimation { duration: 120 } }
            }
        }
    }
}
