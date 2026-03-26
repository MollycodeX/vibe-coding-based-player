import QtQuick

// MeshGradientBackground – animated fluid gradient with color blobs.
// Feed it colors extracted from album art for a living background.
Canvas {
    id: root

    property color color1: "#1a0533"
    property color color2: "#0a1628"
    property color color3: "#0f2027"
    property color color4: "#1a0a2e"
    property real animSpeed: 0.0004

    // Internal blob state
    property real _t: 0

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()

        var w = width, h = height

        // Dark base fill
        ctx.fillStyle = Qt.rgba(0.02, 0.02, 0.06, 1.0)
        ctx.fillRect(0, 0, w, h)

        // Draw soft radial blobs at animated positions
        var blobs = [
            { cx: w * (0.25 + 0.2 * Math.sin(_t * 0.7)),
              cy: h * (0.3 + 0.15 * Math.cos(_t * 0.5)),
              r: Math.max(w, h) * 0.55, color: root.color1 },
            { cx: w * (0.75 + 0.15 * Math.cos(_t * 0.6)),
              cy: h * (0.2 + 0.2 * Math.sin(_t * 0.8)),
              r: Math.max(w, h) * 0.45, color: root.color2 },
            { cx: w * (0.6 + 0.2 * Math.sin(_t * 0.9)),
              cy: h * (0.75 + 0.15 * Math.cos(_t * 0.4)),
              r: Math.max(w, h) * 0.5, color: root.color3 },
            { cx: w * (0.2 + 0.15 * Math.cos(_t * 1.1)),
              cy: h * (0.8 + 0.1 * Math.sin(_t * 0.6)),
              r: Math.max(w, h) * 0.4, color: root.color4 }
        ]

        for (var i = 0; i < blobs.length; i++) {
            var b = blobs[i]
            var grad = ctx.createRadialGradient(b.cx, b.cy, 0, b.cx, b.cy, b.r)
            // Extract RGBA from the Qt color and apply alpha
            var c = b.color
            grad.addColorStop(0, Qt.rgba(c.r, c.g, c.b, 0.45))
            grad.addColorStop(0.4, Qt.rgba(c.r, c.g, c.b, 0.2))
            grad.addColorStop(1, Qt.rgba(c.r, c.g, c.b, 0))
            ctx.fillStyle = grad
            ctx.fillRect(0, 0, w, h)
        }
    }

    Timer {
        running: root.visible
        interval: 50
        repeat: true
        onTriggered: {
            root._t += root.animSpeed * interval
            root.requestPaint()
        }
    }

    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onColor1Changed: requestPaint()
    onColor2Changed: requestPaint()
    onColor3Changed: requestPaint()
    onColor4Changed: requestPaint()
}
