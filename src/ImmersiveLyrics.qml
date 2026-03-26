import QtQuick
import Qt5Compat.GraphicalEffects

// ImmersiveLyrics – depth-based lyrics display where the current line
// is in focus and surrounding lines blur into the background.
Item {
    id: root

    property string lyrics: ""
    property color textColor: "#e0e0e0"
    property color glowColor: "#5599ff"
    property bool expanded: true

    clip: true

    // Split lyrics into lines for depth rendering
    property var _lines: lyrics.split("\n").filter(function(l) { return l.trim() !== "" })
    property int _currentLine: 0

    // Auto-scroll timer (simple approximation – scrolls through lines)
    Timer {
        id: scrollTimer
        interval: 4000
        repeat: true
        running: root.visible && root.expanded && root._lines.length > 1
        onTriggered: {
            root._currentLine = (root._currentLine + 1) % root._lines.length
        }
    }

    onLyricsChanged: _currentLine = 0

    ListView {
        id: lyricsView
        anchors.fill: parent
        model: root._lines
        clip: true
        spacing: 6
        preferredHighlightBegin: height * 0.35
        preferredHighlightEnd: height * 0.65
        highlightRangeMode: ListView.ApplyRange
        currentIndex: root._currentLine

        Behavior on contentY {
            SpringAnimation { spring: 2; damping: 0.5 }
        }

        delegate: Item {
            width: lyricsView.width
            height: lyricText.implicitHeight + 8

            property bool isCurrent: index === lyricsView.currentIndex
            property int dist: Math.abs(index - lyricsView.currentIndex)

            Text {
                id: lyricText
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                text: modelData
                wrapMode: Text.Wrap
                font.pixelSize: isCurrent ? 15 : 13
                font.weight: isCurrent ? Font.DemiBold : Font.Normal
                font.family: "sans-serif"
                color: root.textColor

                opacity: {
                    if (isCurrent) return 1.0
                    if (dist === 1) return 0.55
                    if (dist === 2) return 0.3
                    return 0.15
                }

                Behavior on opacity {
                    NumberAnimation { duration: 400; easing.type: Easing.OutCubic }
                }
                Behavior on font.pixelSize {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }

                // Subtle glow on current line
                layer.enabled: isCurrent
                layer.effect: Glow {
                    radius: 4
                    samples: 9
                    color: Qt.rgba(root.glowColor.r, root.glowColor.g,
                                   root.glowColor.b, 0.3)
                    spread: 0.1
                }
            }

            // Scale effect for depth
            transform: Scale {
                origin.x: parent.width / 2
                origin.y: parent.height / 2
                xScale: isCurrent ? 1.0 : (1.0 - dist * 0.02)
                yScale: xScale
                Behavior on xScale {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }
            }
        }
    }
}
