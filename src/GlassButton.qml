import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

// GlassButton – a skeuomorphic glass/acrylic button with metallic sheen.
Button {
    id: root

    property color baseColor: Qt.rgba(1, 1, 1, 0.08)
    property color hoverColor: Qt.rgba(1, 1, 1, 0.14)
    property color pressColor: Qt.rgba(1, 1, 1, 0.06)
    property color borderColor: Qt.rgba(1, 1, 1, 0.15)
    property color textColor: "#e0e0e0"
    property color accentGlow: "#5599ff"
    property real cornerRadius: 12
    property string iconText: ""   // emoji / unicode icon
    property bool isAccent: false  // whether this is a primary/accent button

    leftPadding: 16
    rightPadding: 16
    topPadding: 8
    bottomPadding: 8

    contentItem: Row {
        spacing: root.iconText !== "" && root.text !== "" ? 6 : 0

        Text {
            visible: root.iconText !== ""
            text: root.iconText
            font.pixelSize: 16
            anchors.verticalCenter: parent.verticalCenter
            color: root.isAccent ? root.accentGlow : root.textColor
        }

        Text {
            visible: root.text !== ""
            text: root.text
            font.pixelSize: 13
            font.family: "sans-serif"
            font.weight: Font.Medium
            color: root.textColor
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        id: bg
        implicitWidth: 60
        implicitHeight: 36
        radius: root.cornerRadius
        color: root.pressed ? root.pressColor
                            : (root.hovered ? root.hoverColor : root.baseColor)

        border.width: 1
        border.color: root.hovered ? Qt.rgba(1, 1, 1, 0.3) : root.borderColor

        Behavior on color {
            ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
        }
        Behavior on border.color {
            ColorAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        // Top edge highlight (metallic sheen)
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 1
            height: 1
            radius: root.cornerRadius
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.3; color: Qt.rgba(1, 1, 1, root.hovered ? 0.25 : 0.1) }
                GradientStop { position: 0.7; color: Qt.rgba(1, 1, 1, root.hovered ? 0.25 : 0.1) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        // Press shadow (inset feel)
        Rectangle {
            anchors.fill: parent
            radius: root.cornerRadius
            color: "transparent"
            opacity: root.pressed ? 0.15 : 0
            border.width: 1
            border.color: Qt.rgba(0, 0, 0, 0.4)
            Behavior on opacity { NumberAnimation { duration: 100 } }
        }

        // Accent glow for primary buttons
        layer.enabled: root.isAccent && root.hovered
        layer.effect: Glow {
            radius: 8
            samples: 17
            color: root.accentGlow
            spread: 0.1
        }
    }

    // Jelly press scale
    scale: root.pressed ? 0.95 : 1.0
    Behavior on scale {
        SpringAnimation { spring: 5; damping: 0.3; mass: 0.3 }
    }
}
