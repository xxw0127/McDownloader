import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// Read-only path field with a browse action. The field is a sunken well rather
// than a white box, so the eye reads it as input and not as another card.
RowLayout {
    id: field

    property alias text: pathField.text
    property string placeholder: ""
    readonly property bool isEmpty: pathField.text.length === 0

    signal browseClicked()

    spacing: 8

    TextField {
        id: pathField
        Layout.fillWidth: true
        Layout.preferredHeight: 36
        implicitHeight: 36
        readOnly: true
        selectByMouse: true
        hoverEnabled: true
        placeholderText: field.placeholder
        placeholderTextColor: "#656D7E"
        leftPadding: 11
        rightPadding: 11
        font.family: "Microsoft YaHei UI"
        font.pixelSize: 12
        color: "#2C3444"
        selectionColor: "#2A6AD9"
        selectedTextColor: "#FFFFFF"
        onTextChanged: cursorPosition = 0

        background: Rectangle {
            radius: 8
            color: pathField.activeFocus ? "#FFFFFF" : (pathField.hovered ? "#EFF2F7" : "#F3F5F8")
            border.width: pathField.activeFocus ? 2 : 1
            border.color: pathField.activeFocus ? "#2A6AD9" : "#E4E8EF"
            Behavior on color { ColorAnimation { duration: 120 } }
            Behavior on border.color { ColorAnimation { duration: 120 } }
        }
    }

    FlatButton {
        text: "浏览"
        variant: "secondary"
        implicitWidth: 62
        implicitHeight: 36
        onClicked: field.browseClicked()
    }
}
