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

    // -----------------------------------------------------------------------
    // Metadata selection dialog – shown when multiple results are returned
    // -----------------------------------------------------------------------
    Dialog {
        id: metadataSelectionDialog
        title: qsTr("Select Metadata")
        modal: true
        anchors.centerIn: parent
        width: Math.min(root.width - 40, 420)
        height: Math.min(root.height - 80, 400)
        standardButtons: Dialog.Cancel

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: qsTr("Multiple results found. Please select the correct one:")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            ListView {
                id: resultsList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: playerController.metadataResults

                delegate: Rectangle {
                    width: resultsList.width
                    height: 56
                    color: resultMouseArea.containsMouse ? "#e8f0fe" : (index % 2 === 0 ? "#ffffff" : "#f8f8f8")
                    radius: 4

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 2

                        Label {
                            text: modelData.title || qsTr("(unknown title)")
                            font.bold: true
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Label {
                            text: (modelData.artist || qsTr("Unknown Artist"))
                                  + (modelData.album ? " — " + modelData.album : "")
                            font.pointSize: 9
                            color: "#666666"
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        id: resultMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            playerController.selectMetadataResult(index)
                            metadataSelectionDialog.close()
                        }
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // Write metadata confirmation dialog
    // -----------------------------------------------------------------------
    Dialog {
        id: writeMetadataDialog
        title: qsTr("Save Metadata")
        modal: true
        anchors.centerIn: parent
        width: Math.min(root.width - 40, 360)
        standardButtons: Dialog.Yes | Dialog.No

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: qsTr("Write the following metadata into the audio file?")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Title: ") + playerController.trackTitle
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Artist: ") + playerController.trackArtist
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Album: ") + playerController.trackAlbum
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }

        onAccepted: playerController.writeMetadataToFile()
    }

    // Listen for metadataResults changes to auto-show selection dialog.
    Connections {
        target: playerController
        function onMetadataResultsChanged() {
            if (playerController.metadataResults.length > 1)
                metadataSelectionDialog.open()
        }
        function onMetadataWritten(success) {
            writeResultLabel.text = success
                ? qsTr("Metadata saved successfully!")
                : qsTr("Failed to save metadata.")
            writeResultLabel.visible = true
            writeResultTimer.restart()
        }
    }

    // Timer to hide the write-result label after a few seconds
    Timer {
        id: writeResultTimer
        interval: 3000
        onTriggered: writeResultLabel.visible = false
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

            // Metadata action buttons
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8
                visible: playerController.trackTitle !== ""

                Button {
                    text: qsTr("Choose Result...")
                    font.pointSize: 9
                    visible: playerController.metadataResults.length > 1
                    onClicked: metadataSelectionDialog.open()
                }

                Button {
                    text: qsTr("Save to File")
                    font.pointSize: 9
                    visible: playerController.metadataWriteSupported
                    onClicked: writeMetadataDialog.open()
                }
            }

            // Fingerprint availability indicator
            Label {
                Layout.alignment: Qt.AlignHCenter
                visible: playerController.fingerprintAvailable && playerController.currentTrack !== ""
                text: "🎵 " + qsTr("Audio fingerprint identification active")
                font.pointSize: 8
                color: "#4CAF50"
            }

            // Write result notification
            Label {
                id: writeResultLabel
                Layout.alignment: Qt.AlignHCenter
                visible: false
                font.pointSize: 9
                color: "#1976D2"
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
