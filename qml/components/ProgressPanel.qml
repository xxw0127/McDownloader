import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "../Theme.js" as T

// Install progress. Hidden entirely unless a job is running, so the panel does
// not carry a dead grey bar through its idle life.
ColumnLayout {
    id: panel

    property int progress: 0
    property string status: ""
    property bool installing: false

    readonly property bool indeterminate: panel.installing && panel.progress <= 0

    spacing: 9

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: panel.status
            font.family: T.fontUi
            font.pixelSize: 11
            color: T.ink2
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }

        Text {
            visible: panel.installing && !panel.indeterminate
            text: Math.round(panel.progress) + "%"
            font.family: T.fontUi
            font.pixelSize: 11
            font.weight: Font.DemiBold
            color: T.ink2
        }
    }

    Rectangle {
        id: track
        Layout.fillWidth: true
        Layout.preferredHeight: 6
        radius: 3
        color: "#E4E8EF"
        clip: true

        Rectangle {
            visible: !panel.indeterminate
            height: parent.height
            radius: 3
            color: T.brand
            width: panel.progress <= 0 ? 0
                                       : Math.max(6, track.width * Math.min(100, panel.progress) / 100)
            Behavior on width {
                NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
            }
        }

        Rectangle {
            id: runner
            visible: panel.indeterminate
            width: 44
            height: parent.height
            radius: 3
            color: T.brand
            x: 0

            SequentialAnimation on x {
                running: runner.visible
                loops: Animation.Infinite
                NumberAnimation {
                    from: 0
                    to: Math.max(0, track.width - runner.width)
                    duration: 880
                    easing.type: Easing.InOutCubic
                }
                NumberAnimation {
                    from: Math.max(0, track.width - runner.width)
                    to: 0
                    duration: 880
                    easing.type: Easing.InOutCubic
                }
            }
        }
    }
}
