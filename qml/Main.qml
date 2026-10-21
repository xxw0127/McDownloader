import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    visible: true
    width: 1040
    height: 720
    minimumWidth: 900
    minimumHeight: 620
    title: app.appName
    color: "#F7F7F8"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Minecraft 版本"
                font.pixelSize: 16
                font.bold: true
                color: "#1F2328"
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: "下载源：自动"
                font.pixelSize: 12
                color: "#6B7280"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FFFFFF"
            radius: 4
            border.width: 1
            border.color: "#D9DDE3"

            Text {
                anchors.centerIn: parent
                text: "版本列表将在此显示"
                font.pixelSize: 13
                color: "#6B7280"
            }
        }
    }
}
