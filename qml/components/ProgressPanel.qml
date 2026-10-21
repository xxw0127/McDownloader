import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: panel

    property int progress: 0
    property string status: ""
    property bool installing: false
    signal cancelClicked()

    spacing: 8

    ProgressBar {
        Layout.fillWidth: true
        from: 0
        to: 100
        value: panel.progress

        background: Rectangle {
            implicitHeight: 6
            radius: 3
            color: "#E5E7EB"
        }
        contentItem: Item {
            implicitHeight: 6
            Rectangle {
                width: panel.progress * parent.width / 100
                height: parent.height
                radius: 3
                color: "#2563EB"
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: panel.status
            font.pixelSize: 11
            color: "#6B7280"
            wrapMode: Text.Wrap
        }

        FlatButton {
            visible: panel.installing
            text: "取消"
            primary: false
            implicitWidth: 64
            onClicked: panel.cancelClicked()
        }
    }
}
