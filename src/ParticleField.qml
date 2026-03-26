import QtQuick

// ParticleField – delicate floating bubbles / light dust rising slowly.
Canvas {
    id: root

    property int particleCount: 35
    property color particleColor: "#ffffff"
    property real maxOpacity: 0.25

    // Internal particle data (populated once)
    property var _particles: []

    Component.onCompleted: _initParticles()

    function _initParticles() {
        var arr = []
        for (var i = 0; i < particleCount; i++) {
            arr.push({
                x: Math.random() * width,
                y: Math.random() * height,
                r: 1 + Math.random() * 2.5,            // radius 1–3.5
                speed: 0.15 + Math.random() * 0.45,     // rise speed
                drift: (Math.random() - 0.5) * 0.3,     // horizontal wander
                alpha: 0.05 + Math.random() * (maxOpacity - 0.05),
                phase: Math.random() * Math.PI * 2
            })
        }
        _particles = arr
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.reset()

        var w = width, h = height
        var c = particleColor
        var t = Date.now() * 0.001

        for (var i = 0; i < _particles.length; i++) {
            var p = _particles[i]

            // Move upward, wrap around
            p.y -= p.speed
            p.x += p.drift + Math.sin(t + p.phase) * 0.15
            if (p.y < -10) { p.y = h + 10; p.x = Math.random() * w }
            if (p.x < -10) p.x = w + 10
            if (p.x > w + 10) p.x = -10

            // Draw soft circle
            var grad = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, p.r * 2)
            grad.addColorStop(0, Qt.rgba(c.r, c.g, c.b, p.alpha))
            grad.addColorStop(0.5, Qt.rgba(c.r, c.g, c.b, p.alpha * 0.4))
            grad.addColorStop(1, Qt.rgba(c.r, c.g, c.b, 0))
            ctx.fillStyle = grad
            ctx.beginPath()
            ctx.arc(p.x, p.y, p.r * 2, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    Timer {
        running: root.visible
        interval: 50
        repeat: true
        onTriggered: root.requestPaint()
    }

    onWidthChanged: { _initParticles(); requestPaint() }
    onHeightChanged: { _initParticles(); requestPaint() }
}
