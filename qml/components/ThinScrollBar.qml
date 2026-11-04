import QtQuick 2.15
import QtQuick.Controls 2.15
import "../Theme.js" as T

// Thin scrollbar that fades in on hover instead of keeping a permanent grey
// channel down the list.
ScrollBar {
    id: bar

    policy: ScrollBar.AsNeeded
    width: 10
    padding: 3

    contentItem: Rectangle {
        implicitWidth: 4
        implicitHeight: 32
        radius: 2
        color: bar.pressed ? "#98A2B4" : "#C4CBD8"
        opacity: bar.pressed || bar.hovered ? 1.0 : 0.85
        Behavior on opacity { NumberAnimation { duration: 160 } }
    }

    background: Item {}
}
