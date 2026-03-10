import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

Window {
    id: root
    width: 480
    height: 700
    minimumWidth: 360
    minimumHeight: 520
    visible: true
    title: qsTr("Vibe Player")

    // Helper to format seconds as m:ss
    function formatTime(secs) {
        if (isNaN(secs) || secs < 0) secs = 0
        var m = Math.floor(secs / 60)
        var s = Math.floor(secs % 60)
        return m + ":" + (s < 10 ? "0" : "") + s
    }

    // Timer to poll playback position while playing
    Timer {
        id: positionTimer
        interval: 250
        repeat: true
        running: playerController.isPlaying
        onTriggered: playerController.updatePosition()
    }

    // File dialog for adding individual audio files
    FileDialog {
        id: fileDialog
        title: qsTr("Select Audio Files")
        nameFilters: [qsTr("Audio files") + " (*.mp3 *.wav *.flac *.ogg *.aac *.wma *.m4a *.opus)"]
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            for (var i = 0; i < selectedFiles.length; i++)
                playerController.addTrackUrl(selectedFiles[i])
        }
    }

    // Folder dialog for adding an entire folder of audio files
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Music Folder")
        onAccepted: playerController.addFolderUrl(selectedFolder)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Current track display with metadata
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: {
                    if (playerController.trackTitle !== "")
                        return playerController.trackTitle
                    if (playerController.currentTrack !== "")
                        return qsTr("Now Playing: ") + playerController.currentTrack
                    return qsTr("No Track Loaded")
                }
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideMiddle
                font.pointSize: 12
                font.bold: true
            }

            Text {
                Layout.fillWidth: true
                visible: playerController.trackArtist !== ""
                text: playerController.trackArtist
                      + (playerController.trackAlbum !== ""
                         ? " — " + playerController.trackAlbum : "")
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideMiddle
                font.pointSize: 10
                color: "#555555"
            }
        }

        // Progress bar
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Slider {
                id: progressSlider
                Layout.fillWidth: true
                from: 0
                to: playerController.duration > 0 ? playerController.duration : 1
                value: playerController.position
                enabled: playerController.duration > 0
                onMoved: playerController.seek(value)
            }

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: formatTime(playerController.position)
                    font.pointSize: 9
                    color: "#666666"
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: formatTime(playerController.duration)
                    font.pointSize: 9
                    color: "#666666"
                }
            }
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

        // Add track buttons (file picker)
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            Button {
                text: qsTr("Add Files...")
                onClicked: fileDialog.open()
            }
            Button {
                text: qsTr("Add Folder...")
                onClicked: folderDialog.open()
            }
        }

        // Lyrics panel (collapsible)
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Lyrics")
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Button {
                    id: lyricsToggle
                    text: lyricsPane.visible ? "▲" : "▼"
                    flat: true
                    font.pointSize: 9
                    implicitWidth: 32
                    Accessible.name: lyricsPane.visible ? qsTr("Collapse lyrics") : qsTr("Expand lyrics")
                    onClicked: lyricsPane.visible = !lyricsPane.visible
                }
            }

            ScrollView {
                id: lyricsPane
                Layout.fillWidth: true
                Layout.preferredHeight: 120
                visible: playerController.lyrics !== ""
                clip: true

                TextArea {
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    text: playerController.lyrics
                    font.pointSize: 10
                    color: "#333333"
                    background: Rectangle {
                        color: "#f8f8f8"
                        radius: 4
                    }
                }
            }

            Label {
                visible: playerController.lyrics === "" && playerController.currentTrack !== ""
                text: qsTr("(no lyrics available)")
                color: "#999999"
                font.pointSize: 9
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
