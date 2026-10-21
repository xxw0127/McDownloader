import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    id: field

    property alias text: pathField.text
    signal browseClicked()

    spacing: 8

    TextField {
        id: pathField
        Layout.fillWidth: true
        readOnly: true
        selectByMouse: true
        font.pixelSize: 12
        color: "#1F2328"
        onTextChanged: cursorPosition = 0
        background: Rectangle {
            radius: 4
            color: "#FFFFFF"
            border.width: 1
            border.color: "#D9DDE3"
        }
    }

    FlatButton {
        text: "浏览..."
        primary: false
        implicitWidth: 76
        onClicked: field.browseClicked()
    }
}
