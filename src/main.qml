import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: root
    width: 480
    height: 320
    minimumWidth: 360
    minimumHeight: 260
    visible: true
    title: qsTr("Vibe Player")

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Current track display
        Text {
            width: parent.width
            text: playerController.currentTrack !== ""
                  ? qsTr("Now Playing: ") + playerController.currentTrack
                  : qsTr("No Track Loaded")
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            font.pointSize: 11
        }

        // Transport controls
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Button {
                text: qsTr("Previous")
                onClicked: playerController.previous()
            }
            Button {
                text: playerController.isPlaying ? qsTr("Pause") : qsTr("Play")
                onClicked: {
                    if (playerController.isPlaying)
                        playerController.pause()
                    else
                        playerController.play()
                }
            }
            Button {
                text: qsTr("Stop")
                onClicked: playerController.stop()
            }
            Button {
                text: qsTr("Next")
                onClicked: playerController.next()
            }
        }

        // Volume control
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Label { text: qsTr("Volume") }
            Slider {
                id: volumeSlider
                from: 0.0
                to: 1.0
                value: playerController.volume
                onMoved: playerController.volume = value
                implicitWidth: 200
            }
            Label { text: Math.round(volumeSlider.value * 100) + "%" }
        }

        // Add track row
        Row {
            id: addTrackRow
            width: parent.width
            spacing: 8

            TextField {
                id: trackPathField
                width: addTrackRow.width - addButton.width - addTrackRow.spacing
                placeholderText: qsTr("Audio file path...")
            }
            Button {
                id: addButton
                text: qsTr("Add")
                onClicked: {
                    if (trackPathField.text !== "") {
                        playerController.addTrack(trackPathField.text)
                        trackPathField.text = ""
                    }
                }
            }
        }
    }
}
