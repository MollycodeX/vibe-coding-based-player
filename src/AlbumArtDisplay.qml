import QtQuick
import Qt5Compat.GraphicalEffects

// AlbumArtDisplay – holographic 3D album art with rotation and reflection.
Item {
    id: root

    property string source: ""
    property bool isPlaying: false
    property real parallaxX: 0   // -1..1, from mouse
    property real parallaxY: 0   // -1..1, from mouse

    // Emitted once the image loads so parent can extract dominant colors
    signal imageReady()
    // Expose dominant colors extracted from the cover
    property color dominantColor1: "#1a0533"
    property color dominantColor2: "#0a1628"
    property color dominantColor3: "#0f2027"
    property color dominantColor4: "#1a0a2e"

    implicitWidth: 200
    implicitHeight: 200

    // 3D perspective wrapper
    Item {
        id: perspectiveContainer
        anchors.centerIn: parent
        width: root.width
        height: root.height

        transform: [
            Rotation {
                origin.x: perspectiveContainer.width / 2
                origin.y: perspectiveContainer.height / 2
                axis { x: 1; y: 0; z: 0 }
                angle: root.parallaxY * 6  // tilt up to ±6°
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.4 }
                }
            },
            Rotation {
                origin.x: perspectiveContainer.width / 2
                origin.y: perspectiveContainer.height / 2
                axis { x: 0; y: 1; z: 0 }
                angle: -root.parallaxX * 6
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.4 }
                }
            }
        ]

        // Shadow beneath the album art
        RectangularGlow {
            anchors.centerIn: artClip
            width: artClip.width + 4
            height: artClip.height + 4
            glowRadius: 20
            spread: 0.1
            color: Qt.rgba(root.dominantColor1.r,
                           root.dominantColor1.g,
                           root.dominantColor1.b, 0.4)
            cornerRadius: artClip.radius + glowRadius
        }

        // Round-clipped album art
        Rectangle {
            id: artClip
            anchors.centerIn: parent
            width: parent.width * 0.85
            height: parent.height * 0.85
            radius: 16
            color: Qt.rgba(1, 1, 1, 0.05)
            clip: true

            Image {
                id: coverImage
                anchors.fill: parent
                source: root.source
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                visible: root.source !== ""

                onStatusChanged: {
                    if (status === Image.Ready) {
                        root._extractColors()
                        root.imageReady()
                    }
                }
            }

            // Placeholder when no cover
            Item {
                anchors.fill: parent
                visible: root.source === "" || coverImage.status !== Image.Ready

                Rectangle {
                    anchors.fill: parent
                    color: Qt.rgba(1, 1, 1, 0.04)
                }

                Text {
                    anchors.centerIn: parent
                    text: "♪"
                    font.pixelSize: 48
                    color: Qt.rgba(1, 1, 1, 0.2)
                }
            }

            // Glass reflection overlay
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.12) }
                    GradientStop { position: 0.35; color: Qt.rgba(1, 1, 1, 0.02) }
                    GradientStop { position: 0.65; color: "transparent" }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.1) }
                }
            }

            // Edge highlight
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                radius: parent.radius
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.15)
            }
        }

        // Slow rotation when playing
        RotationAnimation on rotation {
            from: 0
            to: 360
            duration: 30000   // 30 seconds per revolution
            loops: Animation.Infinite
            running: root.isPlaying && root.source !== ""
            paused: !root.isPlaying
        }
    }

    // --- Color extraction via hidden Canvas ---
    Canvas {
        id: colorExtractor
        width: 8
        height: 8
        visible: false

        onPaint: {
            var ctx = getContext("2d")
            ctx.drawImage(root.source, 0, 0, 8, 8)
        }

        onPainted: {
            var ctx = getContext("2d")
            try {
                var data = ctx.getImageData(0, 0, 8, 8).data
                // Sample corners and center for dominant colors
                var samples = [
                    _sampleAt(data, 8, 1, 1),  // top-left
                    _sampleAt(data, 8, 6, 1),  // top-right
                    _sampleAt(data, 8, 1, 6),  // bottom-left
                    _sampleAt(data, 8, 4, 4)   // center
                ]
                root.dominantColor1 = _toColor(samples[0])
                root.dominantColor2 = _toColor(samples[1])
                root.dominantColor3 = _toColor(samples[2])
                root.dominantColor4 = _toColor(samples[3])
            } catch(e) {
                // Keep defaults on error
            }
        }
    }

    function _extractColors() {
        if (root.source !== "") {
            colorExtractor.loadImage(root.source)
            colorExtractor.requestPaint()
        }
    }

    function _sampleAt(data, stride, x, y) {
        var idx = (y * stride + x) * 4
        return { r: data[idx] / 255, g: data[idx+1] / 255, b: data[idx+2] / 255 }
    }

    function _toColor(s) {
        return Qt.rgba(s.r, s.g, s.b, 1.0)
    }
}
