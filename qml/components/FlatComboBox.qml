import QtQuick 2.15
import QtQuick.Controls 2.15

ComboBox {
    id: control

    implicitHeight: 34
    font.pixelSize: 12

    contentItem: Text {
        leftPadding: 10
        rightPadding: 26
        text: control.displayText
        font: control.font
        color: "#1F2328"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Text {
        x: control.width - width - 10
        y: (control.height - height) / 2
        text: "▾"
        font.pixelSize: 12
        color: "#6B7280"
    }

    background: Rectangle {
        radius: 4
        color: "#FFFFFF"
        border.width: 1
        border.color: control.hovered ? "#9CA3AF" : "#D9DDE3"
    }

    popup: Popup {
        y: control.height + 2
        width: control.width
        implicitHeight: contentItem.implicitHeight + 8
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            radius: 4
            color: "#FFFFFF"
            border.width: 1
            border.color: "#D9DDE3"
        }
    }
}
