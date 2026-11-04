import QtQuick 2.15
import QtQuick.Controls 2.15
import "../Theme.js" as T

// Themed combo box. Used for the loader version list, where the choices run
// into the hundreds and a plain control keeps the scan cheap.
ComboBox {
    id: control

    property string placeholder: "暂无可用版本"

    implicitHeight: 36
    font.family: T.fontUi
    font.pixelSize: 12
    hoverEnabled: true

    contentItem: Text {
        leftPadding: 11
        rightPadding: 32
        text: control.count > 0 ? control.displayText : control.placeholder
        font: control.font
        color: control.count > 0 ? T.ink : T.ink3
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Caret {
        x: control.width - width - 12
        y: (control.height - height) / 2
        expanded: control.popup.visible
        stroke: control.hovered ? T.ink2 : T.ink3
    }

    background: Rectangle {
        radius: 8
        color: "#FFFFFF"
        border.width: 1
        border.color: control.popup.visible ? T.brand
                                            : (control.hovered ? "#C3CBDA" : "#DBE0E9")
        Behavior on border.color { ColorAnimation { duration: 120 } }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 264)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        }

        background: Rectangle {
            radius: 8
            color: "#FFFFFF"
            border.width: 1
            border.color: "#DDE2EA"
        }
    }

    delegate: ItemDelegate {
        id: row

        width: control.width - 8
        height: 30
        highlighted: control.highlightedIndex === index

        contentItem: Text {
            leftPadding: 8
            text: row.text
            font.family: T.fontUi
            font.pixelSize: 12
            color: row.highlighted ? T.brandInk : T.ink
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: 5
            color: row.highlighted ? T.brandSoft : "transparent"
        }
    }

    onCountChanged: {
        if (count > 0 && currentIndex < 0)
            currentIndex = 0
    }
}
