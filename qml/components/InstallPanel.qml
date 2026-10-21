import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: panel

    color: "#FFFFFF"
    radius: 4
    border.width: 1
    border.color: "#D9DDE3"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: app.selectedVersion !== "" ? "安装 " + app.selectedVersion : "安装设置"
            font.pixelSize: 15
            font.bold: true
            color: "#1F2328"
        }

        Text {
            text: "加载器"
            font.pixelSize: 11
            color: "#6B7280"
        }

        FlatComboBox {
            Layout.fillWidth: true
            model: ["原版", "Fabric", "Quilt", "Forge"]
            currentIndex: app.loaderKind === "fabric" ? 1 : (app.loaderKind === "quilt" ? 2 : (app.loaderKind === "forge" ? 3 : 0))
            onActivated: app.loaderKind = index === 1 ? "fabric" : (index === 2 ? "quilt" : (index === 3 ? "forge" : "vanilla"))
        }

        Text {
            visible: app.loaderKind !== "vanilla"
            text: "加载器版本"
            font.pixelSize: 11
            color: "#6B7280"
        }

        FlatComboBox {
            visible: app.loaderKind !== "vanilla"
            Layout.fillWidth: true
            model: app.loaderModel
            textRole: "version"
            currentIndex: 0
            onActivated: app.loaderVersion = currentText
        }

        Text {
            text: "游戏文件夹（.minecraft）"
            font.pixelSize: 11
            color: "#6B7280"
        }

        FolderField {
            Layout.fillWidth: true
            text: app.minecraftDir
            onBrowseClicked: app.chooseMinecraftDir()
        }

        Text {
            Layout.fillWidth: true
            text: app.instanceId() !== "" ? "实例：" + app.instanceId() : ""
            font.pixelSize: 11
            color: "#6B7280"
            elide: Text.ElideRight
        }

        Item { Layout.fillHeight: true }

        FlatButton {
            Layout.fillWidth: true
            text: app.installing ? "正在安装..." : "开始安装"
            enabled: app.selectedVersion !== "" && !app.installing
            onClicked: app.startInstall()
        }

        ProgressPanel {
            Layout.fillWidth: true
            progress: app.progress
            status: app.statusText
            installing: app.installing
            onCancelClicked: app.cancelInstall()
        }
    }
}
