import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: root
    width: 480
    height: 520
    minimumWidth: 360
    minimumHeight: 400
    visible: true
    title: qsTr("Vibe Player")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Current track display
        Text {
            Layout.fillWidth: true
            text: playerController.currentTrack !== ""
                  ? qsTr("Now Playing: ") + playerController.currentTrack
                  : qsTr("No Track Loaded")
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            font.pointSize: 11
        }

        // Transport controls
        Row {
            Layout.alignment: Qt.AlignHCenter
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
            Layout.alignment: Qt.AlignHCenter
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
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: trackPathField
                Layout.fillWidth: true
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

        // Playlist header
        Label {
            text: qsTr("Playlist") + " (" + playerController.trackCount + ")"
            font.bold: true
        }

        // Playlist view
        ListView {
            id: playlistView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: playerController.trackList

            delegate: Rectangle {
                width: playlistView.width
                height: 36
                color: index === playerController.currentIndex ? "#e0e8ff" : (mouseArea.containsMouse ? "#f0f0f0" : "transparent")
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Label {
                        text: (index + 1) + "."
                        Layout.preferredWidth: 30
                        color: index === playerController.currentIndex ? "#3366cc" : "#666666"
                    }
                    Label {
                        text: modelData
                        Layout.fillWidth: true
                        elide: Text.ElideMiddle
                        font.bold: index === playerController.currentIndex
                    }
                    Button {
                        text: qsTr("Remove")
                        flat: true
                        font.pointSize: 9
                        onClicked: playerController.removeTrack(index)
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    // Ignore clicks on the Remove button area
                    acceptedButtons: Qt.LeftButton
                    onDoubleClicked: playerController.selectTrack(index)
                    z: -1
                }
            }

            // Empty-playlist placeholder
            Label {
                anchors.centerIn: parent
                visible: playerController.trackCount === 0
                text: qsTr("(playlist is empty)")
                color: "#999999"
            }
        }
    }
}
