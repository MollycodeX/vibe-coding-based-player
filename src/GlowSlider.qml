import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

// GlowSlider – a luminous progress/volume slider with a glowing handle.
Slider {
    id: root

    property color glowColor: "#5599ff"
    property color trackColor: Qt.rgba(1, 1, 1, 0.1)
    property real trackHeight: 4
    property real handleSize: 16
    property bool springy: true

    background: Item {
        x: root.leftPadding
        y: root.topPadding + root.availableHeight / 2 - root.trackHeight / 2
        width: root.availableWidth
        height: root.trackHeight

        // Track background
        Rectangle {
            anchors.fill: parent
            radius: parent.height / 2
            color: root.trackColor
        }

        // Filled portion (glowing flow line)
        Rectangle {
            width: root.visualPosition * parent.width
            height: parent.height
            radius: parent.height / 2
            color: root.glowColor

            // Glow effect on the filled track
            layer.enabled: true
            layer.effect: Glow {
                radius: 8
                samples: 17
                color: root.glowColor
                spread: 0.3
            }
        }
    }

    handle: Item {
        x: root.leftPadding + root.visualPosition * root.availableWidth - root.handleSize / 2
        y: root.topPadding + root.availableHeight / 2 - root.handleSize / 2
        width: root.handleSize
        height: root.handleSize

        // Spring animation on x position
        Behavior on x {
            enabled: root.springy && !root.pressed
            SpringAnimation {
                spring: 3
                damping: 0.25
                mass: 0.5
            }
        }

        // Outer glow
        Rectangle {
            id: handleGlow
            anchors.centerIn: parent
            width: root.handleSize * (root.pressed ? 2.2 : 1.6)
            height: width
            radius: width / 2
            color: "transparent"

            Behavior on width {
                SpringAnimation { spring: 4; damping: 0.3 }
            }

            RadialGradient {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Qt.rgba(root.glowColor.r,
                                                                   root.glowColor.g,
                                                                   root.glowColor.b,
                                                                   root.pressed ? 0.5 : 0.3) }
                    GradientStop { position: 1.0; color: "transparent" }
                }
            }
        }

        // Handle body (water-drop style)
        Rectangle {
            anchors.centerIn: parent
            width: root.handleSize
            height: root.handleSize
            radius: root.handleSize / 2
            color: root.glowColor

            // Inner highlight
            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.5
                height: parent.height * 0.5
                radius: width / 2
                color: Qt.lighter(root.glowColor, 1.6)
                opacity: 0.6
            }

            // Glow
            layer.enabled: true
            layer.effect: Glow {
                radius: 6
                samples: 13
                color: root.glowColor
                spread: 0.2
            }

            scale: root.pressed ? 1.2 : (root.hovered ? 1.1 : 1.0)
            Behavior on scale {
                SpringAnimation { spring: 5; damping: 0.3 }
            }
        }
    }
}
