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
    color: theme.background

    // Theme toggle: false = light, true = dark
    property bool darkTheme: false

    // Centralized theme palette
    QtObject {
        id: theme
        readonly property color background:        root.darkTheme ? "#1e1e1e" : "#ffffff"
        readonly property color text:              root.darkTheme ? "#e0e0e0" : "#000000"
        readonly property color secondaryText:     root.darkTheme ? "#aaaaaa" : "#555555"
        readonly property color mutedText:         root.darkTheme ? "#888888" : "#666666"
        readonly property color placeholderText:   root.darkTheme ? "#666666" : "#999999"
        readonly property color accent:            root.darkTheme ? "#5599ff" : "#3366cc"
        readonly property color accentBg:          root.darkTheme ? "#2a3a5c" : "#e0e8ff"
        readonly property color hoverBg:           root.darkTheme ? "#2c2c2c" : "#f0f0f0"
        readonly property color surfaceBg:         root.darkTheme ? "#2a2a2a" : "#f8f8f8"
        readonly property color border:            root.darkTheme ? "#444444" : "#cccccc"
        readonly property color success:           root.darkTheme ? "#66bb6a" : "#4CAF50"
        readonly property color info:              root.darkTheme ? "#42a5f5" : "#1976D2"
        readonly property color metadataEvenRow:   root.darkTheme ? "#252525" : "#ffffff"
        readonly property color metadataHoverRow:  root.darkTheme ? "#33404d" : "#e8f0fe"
        readonly property color buttonBg:          root.darkTheme ? "#3a3a3a" : "#e8e8e8"
        readonly property color buttonHoverBg:     root.darkTheme ? "#4a4a4a" : "#d0d0d0"
    }

    // Propagate theme colors to all controls via the palette
    palette.button:          theme.buttonBg
    palette.buttonText:      theme.text
    palette.window:          theme.background
    palette.windowText:      theme.text
    palette.base:            theme.surfaceBg
    palette.text:            theme.text
    palette.highlight:       theme.accent
    palette.highlightedText: root.darkTheme ? "#ffffff" : "#ffffff"

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
        width: Math.min(root.width - 40, 480)
        height: Math.min(root.height - 60, 520)
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
                    height: 64
                    color: resultMouseArea.containsMouse ? theme.metadataHoverRow : (index % 2 === 0 ? theme.metadataEvenRow : theme.surfaceBg)
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 8

                        // Album cover art thumbnail
                        Image {
                            Layout.preferredWidth: 48
                            Layout.preferredHeight: 48
                            source: modelData.coverArtUrl
                            fillMode: Image.PreserveAspectFit
                            asynchronous: true
                            // Placeholder when no cover art or loading failed
                            Rectangle {
                                anchors.fill: parent
                                visible: parent.status !== Image.Ready
                                color: theme.surfaceBg
                                border.color: theme.border
                                border.width: 1
                                radius: 4
                                Label {
                                    anchors.centerIn: parent
                                    text: "🎵"
                                    font.pointSize: 16
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
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
                                color: theme.mutedText
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
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

            // Album cover art preview in dialog
            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 100
                Layout.preferredHeight: 100
                visible: playerController.albumArtUrl !== ""
                source: playerController.albumArtUrl
                fillMode: Image.PreserveAspectFit
                cache: false
            }

            // Checkbox to embed album art
            CheckBox {
                id: embedAlbumArtCheckBox
                text: qsTr("Embed album cover art")
                visible: playerController.albumArtUrl !== ""
                checked: playerController.embedAlbumArt
                onCheckedChanged: playerController.embedAlbumArt = checked
            }
        }

        onAccepted: playerController.writeMetadataToFile()
    }

    // -----------------------------------------------------------------------
    // TagLib-not-available information dialog
    // -----------------------------------------------------------------------
    Dialog {
        id: taglibMissingDialog
        title: qsTr("Feature Unavailable")
        modal: true
        anchors.centerIn: parent
        width: Math.min(root.width - 40, 360)
        standardButtons: Dialog.Ok

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: qsTr("Metadata writing is not available in this build because TagLib was not found at compile time.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("To enable this feature, install TagLib (e.g. libtag1-dev on Ubuntu) and rebuild the application.")
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: theme.mutedText
            }
        }
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

        // Theme and language toggles
        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: translationManager.language === "zh" ? "English" : "中文"
                flat: true
                font.pointSize: 9
                Accessible.name: translationManager.language === "zh"
                    ? qsTr("Switch to English") : qsTr("Switch to Chinese")
                onClicked: translationManager.setLanguage(
                    translationManager.language === "zh" ? "en" : "zh")
            }
            Button {
                text: root.darkTheme ? qsTr("☀ Light") : qsTr("🌙 Dark")
                flat: true
                font.pointSize: 9
                Accessible.name: root.darkTheme ? qsTr("Switch to light theme") : qsTr("Switch to dark theme")
                onClicked: root.darkTheme = !root.darkTheme
            }
        }

        // Current track display with metadata
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            // Album cover art display
            Image {
                id: albumArtImage
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 160
                Layout.preferredHeight: 160
                visible: playerController.albumArtUrl !== ""
                source: playerController.albumArtUrl
                fillMode: Image.PreserveAspectFit
                cache: false

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: theme.border
                    border.width: 1
                    radius: 4
                }
            }

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
                color: theme.text
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
                color: theme.secondaryText
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
                    onClicked: {
                        if (playerController.metadataWriteSupported)
                            writeMetadataDialog.open()
                        else
                            taglibMissingDialog.open()
                    }
                }
            }

            // Fingerprint availability indicator
            Label {
                Layout.alignment: Qt.AlignHCenter
                visible: playerController.fingerprintAvailable && playerController.currentTrack !== ""
                text: "🎵 " + qsTr("Audio fingerprint identification active")
                font.pointSize: 8
                color: theme.success
            }

            // Write result notification
            Label {
                id: writeResultLabel
                Layout.alignment: Qt.AlignHCenter
                visible: false
                font.pointSize: 9
                color: theme.info
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
                    color: theme.mutedText
                }
                Item { Layout.fillWidth: true }
                Label {
                    text: formatTime(playerController.duration)
                    font.pointSize: 9
                    color: theme.mutedText
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
                    color: theme.text
                    background: Rectangle {
                        color: theme.surfaceBg
                        radius: 4
                    }
                }
            }

            Label {
                visible: playerController.lyrics === "" && playerController.currentTrack !== ""
                text: qsTr("(no lyrics available)")
                color: theme.placeholderText
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
                color: index === playerController.currentIndex ? theme.accentBg : (mouseArea.containsMouse ? theme.hoverBg : "transparent")
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Label {
                        text: (index + 1) + "."
                        Layout.preferredWidth: 30
                        color: index === playerController.currentIndex ? theme.accent : theme.mutedText
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
                color: theme.placeholderText
            }
        }
    }
}
