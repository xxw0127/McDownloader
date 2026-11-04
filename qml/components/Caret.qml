import QtQuick 2.15

// A drawn chevron. Replaces the ▴/▾ text glyphs so the whole UI uses one
// consistent icon weight.
Item {
    id: caret

    property bool expanded: false
    property color stroke: "#7C8496"

    implicitWidth: 12
    implicitHeight: 12

    rotation: caret.expanded ? 180 : 0
    Behavior on rotation {
        NumberAnimation { duration: 170; easing.type: Easing.OutCubic }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = caret.stroke
            ctx.lineWidth = 1.7
            ctx.lineCap = "round"
            ctx.lineJoin = "round"
            var cx = width / 2
            var cy = height / 2
            ctx.beginPath()
            ctx.moveTo(cx - 3.4, cy - 1.8)
            ctx.lineTo(cx, cy + 1.6)
            ctx.lineTo(cx + 3.4, cy - 1.8)
            ctx.stroke()
        }
    }

    onStrokeChanged: canvas.requestPaint()
    Component.onCompleted: canvas.requestPaint()
}
