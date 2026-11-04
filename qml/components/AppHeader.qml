import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// Application shell. The dark bar gives the window a committed frame and
// carries the only place where the source of the version data is stated.
Rectangle {
    id: header

    property string sourceLabel: "自动"

    implicitHeight: 60
    color: T.shell

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 20
        spacing: 0

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 32
            implicitHeight: 32
            radius: 7
            color: "#26314A"

            Image {
                anchors.centerIn: parent
                source: "qrc:/images/logo_m.png"
                sourceSize.width: 27
                sourceSize.height: 24
                width: 27
                height: 24
                fillMode: Image.PreserveAspectFit
                smooth: false
            }
        }

        Item { implicitWidth: 11 }

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: "McDownloader"
            font.family: T.fontUi
            font.pixelSize: 14
            font.weight: Font.DemiBold
            font.letterSpacing: 0.2
            color: T.onShell
        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            implicitWidth: 1
            implicitHeight: 13
            color: T.shellRule
        }

        Text {
            Layout.alignment: Qt.AlignVCenter
            text: "Minecraft 原版安装器"
            font.family: T.fontUi
            font.pixelSize: 12
            color: T.onShellDim
        }

        Item { Layout.fillWidth: true }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitHeight: 26
            implicitWidth: pill.implicitWidth + 22
            radius: 13
            color: "#14FFFFFF"

            RowLayout {
                id: pill
                anchors.centerIn: parent
                spacing: 6

                Rectangle {
                    implicitWidth: 6
                    implicitHeight: 6
                    radius: 3
                    color: "#6DBE58"
                }

                Text {
                    text: "下载源 · " + header.sourceLabel
                    font.family: T.fontUi
                    font.pixelSize: 11
                    color: "#C2CBDB"
                }
            }
        }
    }
}
