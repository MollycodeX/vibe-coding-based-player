import QtQuick
import Qt5Compat.GraphicalEffects

// GlassPanel – frosted glass container with blur, tint, and edge lighting.
// Requires a `backgroundSource` Item reference to blur behind the panel.
Item {
    id: root

    property Item backgroundSource: null
    property real blurRadius: 40
    property color tintColor: Qt.rgba(1, 1, 1, 0.06)
    property real borderOpacity: 0.12
    property real cornerRadius: 16
    property bool hovered: false

    // Allow children to be placed inside the panel
    default property alias contentData: contentContainer.data

    // --- Frosted glass background ---
    Item {
        id: blurLayer
        anchors.fill: parent
        visible: root.backgroundSource !== null

        // Round-rect clip mask (rendered off-screen)
        Rectangle {
            id: clipMask
            anchors.fill: parent
            radius: root.cornerRadius
            visible: false
        }

        // Captured + blurred background
        Item {
            id: blurContent
            anchors.fill: parent
            layer.enabled: true
            layer.effect: OpacityMask { maskSource: clipMask }

            ShaderEffectSource {
                id: bgCapture
                anchors.fill: parent
                sourceItem: root.backgroundSource
                sourceRect: {
                    if (!root.backgroundSource) return Qt.rect(0, 0, 0, 0)
                    var p = root.mapToItem(root.backgroundSource, 0, 0)
                    return Qt.rect(p.x, p.y, root.width, root.height)
                }
                live: true
                visible: false
            }

            FastBlur {
                anchors.fill: parent
                source: bgCapture
                radius: root.blurRadius
            }

            // Tint overlay
            Rectangle {
                anchors.fill: parent
                color: root.tintColor
            }
        }
    }

    // Fallback when no background source: solid semi-transparent fill
    Rectangle {
        anchors.fill: parent
        visible: root.backgroundSource === null
        color: root.tintColor
        radius: root.cornerRadius
    }

    // --- Edge highlight border ---
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        radius: root.cornerRadius
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, root.hovered ? root.borderOpacity * 2.5
                                                      : root.borderOpacity)
        Behavior on border.color {
            ColorAnimation { duration: 300; easing.type: Easing.OutCubic }
        }
    }

    // --- Subtle inner glow on hover ---
    Rectangle {
        anchors.fill: parent
        radius: root.cornerRadius
        color: "transparent"
        border.width: 0
        opacity: root.hovered ? 0.06 : 0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        layer.enabled: root.hovered
        layer.effect: Glow {
            radius: 12
            samples: 25
            color: Qt.rgba(1, 1, 1, 0.3)
            spread: 0.1
        }
    }

    // Content container
    Item {
        id: contentContainer
        anchors.fill: parent
    }
}
