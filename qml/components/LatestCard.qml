import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../Theme.js" as T

// The two newest versions, side by side in one card so the top of the list is
// a decision, not decoration.
Rectangle {
    id: card

    property var release: ({})
    property var snapshot: ({})

    readonly property bool hasRelease: card.release && card.release.id !== undefined && card.release.id !== ""
    readonly property bool hasSnapshot: card.snapshot && card.snapshot.id !== undefined && card.snapshot.id !== ""

    implicitHeight: 92
    radius: 12
    color: T.surface
    border.width: 1
    border.color: T.line

    RowLayout {
        anchors.fill: parent
        spacing: 0

        LatestEntry {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: card.hasRelease
            displayId: card.hasRelease ? card.release.displayId : ""
            releaseTime: card.hasRelease ? card.release.releaseTime : ""
            iconSource: card.hasRelease && card.release.icon !== undefined ? card.release.icon : ""
            channel: "最新正式版"
            channelFill: T.brandSoft
            channelInk: T.brandInk
            onClicked: app.selectedVersion = card.release.id
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.topMargin: 20
            Layout.bottomMargin: 20
            implicitWidth: 1
            color: T.line
            visible: card.hasRelease && card.hasSnapshot
        }

        LatestEntry {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: card.hasSnapshot
            displayId: card.hasSnapshot ? card.snapshot.displayId : ""
            releaseTime: card.hasSnapshot ? card.snapshot.releaseTime : ""
            iconSource: card.hasSnapshot && card.snapshot.icon !== undefined ? card.snapshot.icon : ""
            channel: "最新快照"
            channelFill: T.amberSoft
            channelInk: T.amber
            onClicked: app.selectedVersion = card.snapshot.id
        }
    }
}
