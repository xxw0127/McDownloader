import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// Right rail: what is about to be installed, where it goes, and the one action
// that commits. The panel is a column of a single surface, not a floating card.
Rectangle {
    id: panel

    readonly property var info: app.selectedInfo
    readonly property bool hasSelection: app.selectedVersion !== ""
    readonly property string groupName: {
        if (!panel.hasSelection || panel.info === undefined || panel.info.groupName === undefined)
            return ""
        return panel.info.groupName
    }
    readonly property string releaseTime: {
        if (!panel.hasSelection || panel.info === undefined || panel.info.releaseTime === undefined)
            return ""
        return panel.info.releaseTime
    }
    readonly property string iconSource: {
        if (!panel.hasSelection || panel.info === undefined || panel.info.icon === undefined)
            return ""
        return panel.info.icon
    }

    color: T.surface

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 26
        anchors.rightMargin: 26
        anchors.topMargin: 24
        anchors.bottomMargin: 24
        spacing: 0

        // ---------------------------------------------------------------- head
        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Rectangle {
                Layout.alignment: Qt.AlignTop
                implicitWidth: 56
                implicitHeight: 56
                radius: 12
                color: T.surfaceSunken
                border.width: 1
                border.color: T.lineSoft
                visible: panel.iconSource !== ""

                Image {
                    anchors.centerIn: parent
                    source: panel.iconSource
                    sourceSize.width: 48
                    sourceSize.height: 48
                    width: 48
                    height: 48
                    fillMode: Image.PreserveAspectFit
                    smooth: false
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 5

                Text {
                    Layout.fillWidth: true
                    text: panel.hasSelection ? app.selectedVersion : "尚未选择版本"
                    font.family: T.fontUi
                    font.pixelSize: panel.hasSelection ? 23 : 17
                    font.weight: panel.hasSelection ? Font.Bold : Font.DemiBold
                    color: panel.hasSelection ? T.ink : T.ink3
                    elide: Text.ElideRight
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 7
                    visible: panel.hasSelection

                    Badge {
                        label: panel.groupName
                        fill: panel.groupName === "预览版" ? T.amberSoft : T.brandSoft
                        ink: panel.groupName === "预览版" ? T.amber : T.brandInk
                    }

                    Badge {
                        visible: app.loaderKind !== "vanilla"
                        label: T.loaderLabel(app.loaderKind)
                        fill: T.surfaceSunken
                        ink: T.ink2
                    }

                    Text {
                        text: panel.releaseTime
                        font.family: T.fontUi
                        font.pixelSize: 11
                        color: T.ink4
                    }

                    Item { Layout.fillWidth: true }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 14
            visible: !panel.hasSelection
            text: "在左侧列表里挑一个版本，这里会显示安装摘要。"
            font.family: T.fontUi
            font.pixelSize: 12
            color: T.ink4
            wrapMode: Text.Wrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.topMargin: 20
            Layout.bottomMargin: 18
            color: T.line
        }

        LoaderSection {
            Layout.fillWidth: true
        }

        // -------------------------------------------------------------- inputs
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20
            spacing: 18

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 7

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Text {
                        text: "游戏文件夹"
                        font.family: T.fontUi
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        color: T.ink2
                    }

                    Text {
                        text: ".minecraft"
                        font.family: T.fontUi
                        font.pixelSize: 11
                        color: T.ink4
                    }

                    Item { Layout.fillWidth: true }
                }

                FolderField {
                    Layout.fillWidth: true
                    text: app.minecraftDir
                    onBrowseClicked: app.chooseMinecraftDir()
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 7

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6

                    Text {
                        text: "Java 运行时"
                        font.family: T.fontUi
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        color: T.ink2
                    }

                    Text {
                        text: "留空则自动选择"
                        font.family: T.fontUi
                        font.pixelSize: 11
                        color: T.ink4
                    }

                    Item { Layout.fillWidth: true }
                }

                FolderField {
                    Layout.fillWidth: true
                    placeholder: "未指定，安装时自动扫描"
                    text: app.javaPath
                    onBrowseClicked: app.chooseJavaPath()
                }
            }
        }

        // ------------------------------------------------------------ target
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 22
            spacing: 6
            visible: panel.hasSelection

            Text {
                text: "安装到"
                font.family: T.fontUi
                font.pixelSize: 11
                font.weight: Font.DemiBold
                font.letterSpacing: 0.5
                color: T.ink4
            }

            Text {
                Layout.fillWidth: true
                text: {
                    var base = app.minecraftDir
                    if (base.length === 0 || app.selectedVersion === "")
                        return ""
                    // Touch the loader properties so this re-evaluates when the
                    // instance name changes, not only when the MC version does.
                    var kind = app.loaderKind
                    var loaderVersion = app.loaderVersion
                    if (kind !== "vanilla" && loaderVersion === "")
                        return "正在获取加载器版本…"
                    if (base.charAt(base.length - 1) !== "/" && base.charAt(base.length - 1) !== "\\")
                        base = base + "\\"
                    return base + "versions\\" + app.instanceId()
                }
                font.family: "Consolas"
                font.pixelSize: 11
                color: T.ink3
                wrapMode: Text.NoWrap
                elide: Text.ElideMiddle
            }

            Text {
                Layout.fillWidth: true
                Layout.topMargin: 2
                text: "缺失的文件夹会自动创建；重装同一版本前会先备份到 .mcdownloader-backup。"
                font.family: T.fontUi
                font.pixelSize: 11
                color: T.ink4
                wrapMode: Text.Wrap
                lineHeight: 1.5
            }
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.topMargin: 8
            Layout.bottomMargin: 16
            color: T.lineSoft
        }

        // -------------------------------------------------------------- foot
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            ProgressPanel {
                Layout.fillWidth: true
                visible: app.installing
                progress: app.progress
                status: app.statusText
                installing: app.installing
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6
                visible: !app.installing

                Rectangle {
                    Layout.alignment: Qt.AlignVCenter
                    implicitWidth: 5
                    implicitHeight: 5
                    radius: 3
                    color: T.statusTone(app.statusText)
                }

                Text {
                    Layout.fillWidth: true
                    text: app.statusText
                    font.family: T.fontUi
                    font.pixelSize: 11
                    color: T.statusTone(app.statusText)
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                }
            }

            FlatButton {
                Layout.fillWidth: true
                implicitHeight: 44
                radius: 10
                textSize: 13
                text: app.installing ? "取消安装" : "开始安装"
                variant: app.installing ? "secondary" : "primary"
                enabled: app.installing || panel.hasSelection
                onClicked: {
                    if (app.installing)
                        app.cancelInstall()
                    else
                        app.startInstall()
                }
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "安装完成后，用 PCL2 启动这个实例"
                font.family: T.fontUi
                font.pixelSize: 11
                color: T.ink4
            }
        }
    }
}
