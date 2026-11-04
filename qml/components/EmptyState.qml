import QtQuick 2.15
import "../Theme.js" as T

// Shown inside the list area while the manifest is empty: either still loading
// or genuinely nothing to show. The rotating block is the app's only ambient
// motion, borrowed from the game's own loading beat.
Item {
    id: empty

    property string text: ""

    Column {
        anchors.centerIn: parent
        spacing: 14

        Item {
            width: 26
            height: 26
            anchors.horizontalCenter: parent.horizontalCenter

            RotationAnimator on rotation {
                from: 0
                to: 360
                duration: 1100
                loops: Animation.Infinite
                running: empty.visible
            }

            Rectangle {
                width: 11
                height: 11
                radius: 2
                x: 4
                y: 4
                color: T.brand
                opacity: 0.85
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, 280)
            horizontalAlignment: Text.AlignHCenter
            text: empty.text.length > 0 ? empty.text : "暂无版本"
            font.family: T.fontUi
            font.pixelSize: 12
            color: T.ink3
            wrapMode: Text.Wrap
        }
    }
}
