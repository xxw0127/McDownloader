import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "components"
import "Theme.js" as T

Window {
    id: root
    visible: true
    width: 1100
    height: 740
    minimumWidth: 980
    minimumHeight: 640
    title: app.appName
    color: T.ground

    onClosing: {
        if (app.installing) {
            close.accepted = false
            app.cancelInstall()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        AppHeader {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ------------------------------------------------------ version list
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    anchors.rightMargin: 22
                    anchors.topMargin: 18
                    anchors.bottomMargin: 20
                    spacing: 14

                    LatestCard {
                        Layout.fillWidth: true
                        release: app.latestRelease
                        snapshot: app.latestSnapshot
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 2
                        Layout.leftMargin: 2
                        Layout.rightMargin: 14
                        spacing: 10

                        Text {
                            Layout.alignment: Qt.AlignVCenter
                            text: "全部版本"
                            font.family: T.fontUi
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.7
                            color: T.ink4
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            implicitHeight: 1
                            color: T.line
                        }

                        Text {
                            Layout.alignment: Qt.AlignVCenter
                            text: app.versionCount
                            font.family: T.fontUi
                            font.pixelSize: 11
                            color: T.ink4
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ListView {
                            id: versionList
                            anchors.fill: parent
                            clip: true
                            spacing: 0
                            model: app.versionModel
                            boundsBehavior: Flickable.StopAtBounds
                            ScrollBar.vertical: ThinScrollBar {
                                size: versionList.visibleArea.heightRatio
                                visible: versionList.contentHeight > versionList.height + 2
                            }

                            delegate: Item {
                                width: ListView.view.width
                                height: model.header ? 44 : 56

                                SectionCard {
                                    visible: model.header
                                    anchors.fill: parent
                                    label: model.header ? model.groupName : ""
                                    count: model.header ? model.count : 0
                                    expanded: model.header ? model.expanded : false
                                    onToggled: app.versionModel.toggleGroup(model.group)
                                }

                                VersionRow {
                                    visible: !model.header
                                    anchors.fill: parent
                                    displayId: model.header ? "" : model.displayId
                                    releaseTime: model.header ? "" : model.releaseTime
                                    iconSource: model.header ? "" : model.icon
                                    selected: model.header ? false : model.selected
                                    onClicked: app.selectedVersion = model.versionId
                                }
                            }
                        }

                        EmptyState {
                            anchors.fill: parent
                            visible: versionList.count === 0
                            text: app.statusText
                        }
                    }
                }
            }

            // ------------------------------------------------------ install rail
            Item {
                Layout.preferredWidth: 366
                Layout.fillHeight: true

                Rectangle {
                    width: 1
                    height: parent.height
                    color: T.line
                }

                InstallPanel {
                    anchors.fill: parent
                    anchors.leftMargin: 1
                }
            }
        }
    }
}
