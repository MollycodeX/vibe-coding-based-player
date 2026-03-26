import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

Window {
    id: root
    width: 520
    height: 800
    minimumWidth: 400
    minimumHeight: 600
    visible: true
    title: qsTr("Vibe Player")
    color: "#050510"

    // Theme toggle: false = dark (default for Aero), true = light
    property bool darkTheme: true

    // Parallax mouse tracking (-1..1 range)
    property real parallaxX: 0
    property real parallaxY: 0

    // -----------------------------------------------------------------------
    // Next-Gen Aero theme palette
    // -----------------------------------------------------------------------
    QtObject {
        id: theme
        // Base colors
        readonly property color background:        root.darkTheme ? "#050510" : "#dde1ea"
        readonly property color text:              root.darkTheme ? "#e8eaef" : "#1a1a2e"
        readonly property color secondaryText:     root.darkTheme ? Qt.rgba(1,1,1,0.55) : Qt.rgba(0,0,0,0.5)
        readonly property color mutedText:         root.darkTheme ? Qt.rgba(1,1,1,0.3) : Qt.rgba(0,0,0,0.35)
        readonly property color placeholderText:   root.darkTheme ? Qt.rgba(1,1,1,0.2) : Qt.rgba(0,0,0,0.25)
        // Accent derived from album art (or default blue)
        readonly property color accent:            albumArtView.dominantColor2 !== "#0a1628"
                                                       ? albumArtView.dominantColor2
                                                       : (root.darkTheme ? "#5599ff" : "#3366cc")
        readonly property color accentBg:          Qt.rgba(theme.accent.r, theme.accent.g,
                                                           theme.accent.b, root.darkTheme ? 0.15 : 0.2)
        readonly property color hoverBg:           root.darkTheme ? Qt.rgba(1,1,1,0.06) : Qt.rgba(0,0,0,0.05)
        readonly property color surfaceBg:         root.darkTheme ? Qt.rgba(1,1,1,0.04) : Qt.rgba(1,1,1,0.5)
        readonly property color border:            root.darkTheme ? Qt.rgba(1,1,1,0.1) : Qt.rgba(0,0,0,0.1)
        readonly property color success:           "#66bb6a"
        readonly property color info:              "#42a5f5"
        readonly property color metadataEvenRow:   root.darkTheme ? Qt.rgba(1,1,1,0.03) : Qt.rgba(1,1,1,0.7)
        readonly property color metadataHoverRow:  root.darkTheme ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0.9,0.95,1,0.8)
        readonly property color buttonBg:          root.darkTheme ? Qt.rgba(1,1,1,0.08) : Qt.rgba(1,1,1,0.5)
        readonly property color buttonHoverBg:     root.darkTheme ? Qt.rgba(1,1,1,0.14) : Qt.rgba(1,1,1,0.7)

        // Glass panel styling
        readonly property color glassTint:    root.darkTheme ? Qt.rgba(0.05,0.05,0.12,0.55) : Qt.rgba(1,1,1,0.55)
        readonly property real  glassBorder:  root.darkTheme ? 0.12 : 0.2
        readonly property real  glassRadius:  20
        readonly property real  glassBlur:    40
    }

    // Propagate palette
    palette.button:          theme.buttonBg
    palette.buttonText:      theme.text
    palette.window:          theme.background
    palette.windowText:      theme.text
    palette.base:            theme.surfaceBg
    palette.text:            theme.text
    palette.highlight:       theme.accent
    palette.highlightedText: "#ffffff"

    // Helper to format seconds as m:ss
    function formatTime(secs) {
        if (isNaN(secs) || secs < 0) secs = 0
        var m = Math.floor(secs / 60)
        var s = Math.floor(secs % 60)
        return m + ":" + (s < 10 ? "0" : "") + s
    }

    // Position polling timer
    Timer {
        id: positionTimer
        interval: 250
        repeat: true
        running: playerController.isPlaying
        onTriggered: playerController.updatePosition()
    }

    // -----------------------------------------------------------------------
    // File / Folder dialogs (unchanged functionality)
    // -----------------------------------------------------------------------
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
    FolderDialog {
        id: folderDialog
        title: qsTr("Select Music Folder")
        onAccepted: playerController.addFolderUrl(selectedFolder)
    }

    // -----------------------------------------------------------------------
    // Metadata selection dialog (glass-themed)
    // -----------------------------------------------------------------------
    Loader {
        id: metadataSelectionDialogLoader
        active: false
        sourceComponent: Dialog {
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
                    color: theme.text
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
                        color: resultMouseArea.containsMouse
                               ? theme.metadataHoverRow
                               : (index % 2 === 0 ? theme.metadataEvenRow : theme.surfaceBg)
                        radius: 8

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 8

                            Image {
                                Layout.preferredWidth: 48
                                Layout.preferredHeight: 48
                                source: modelData.coverArtUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                Rectangle {
                                    anchors.fill: parent
                                    visible: parent.status !== Image.Ready
                                    color: theme.surfaceBg
                                    border.color: theme.border
                                    border.width: 1
                                    radius: 6
                                    Label {
                                        anchors.centerIn: parent
                                        text: "♪"
                                        font.pixelSize: 20
                                        color: theme.mutedText
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
                                    color: theme.text
                                }
                                Label {
                                    text: (modelData.artist || qsTr("Unknown Artist"))
                                          + (modelData.album ? " — " + modelData.album : "")
                                    font.pixelSize: 11
                                    color: theme.secondaryText
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
        function show() { active = true; item.open() }
    }

    // -----------------------------------------------------------------------
    // Write metadata dialog
    // -----------------------------------------------------------------------
    Loader {
        id: writeMetadataDialogLoader
        active: false
        sourceComponent: Dialog {
            id: writeMetadataDialog
            title: qsTr("Save Metadata")
            modal: true
            anchors.centerIn: parent
            width: Math.min(root.width - 40, 360)
            standardButtons: Dialog.Yes | Dialog.No

            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                Label { text: qsTr("Write the following metadata into the audio file?"); wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.text }
                Label { text: qsTr("Title: ") + playerController.trackTitle; wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.text }
                Label { text: qsTr("Artist: ") + playerController.trackArtist; wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.text }
                Label { text: qsTr("Album: ") + playerController.trackAlbum; wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.text }

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 100; Layout.preferredHeight: 100
                    visible: playerController.albumArtUrl !== ""
                    source: playerController.albumArtUrl
                    fillMode: Image.PreserveAspectFit
                }
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
        function show() { active = true; item.open() }
    }

    // -----------------------------------------------------------------------
    // TagLib-missing dialog
    // -----------------------------------------------------------------------
    Loader {
        id: taglibMissingDialogLoader
        active: false
        sourceComponent: Dialog {
            id: taglibMissingDialog
            title: qsTr("Feature Unavailable")
            modal: true
            anchors.centerIn: parent
            width: Math.min(root.width - 40, 360)
            standardButtons: Dialog.Ok
            ColumnLayout {
                anchors.fill: parent; spacing: 8
                Label { text: qsTr("Metadata writing is not available in this build because TagLib was not found at compile time."); wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.text }
                Label { text: qsTr("To enable this feature, install TagLib (e.g. libtag1-dev on Ubuntu) and rebuild the application."); wrapMode: Text.Wrap; Layout.fillWidth: true; color: theme.mutedText }
            }
        }
        function show() { active = true; item.open() }
    }

    // -----------------------------------------------------------------------
    // Metadata / write-result connections
    // -----------------------------------------------------------------------
    Connections {
        target: playerController
        function onMetadataResultsChanged() {
            if (playerController.metadataResults.length > 1)
                metadataSelectionDialogLoader.show()
        }
        function onMetadataWritten(success) {
            writeResultLabel.text = success
                ? qsTr("Metadata saved successfully!")
                : qsTr("Failed to save metadata.")
            writeResultLabel.visible = true
            writeResultTimer.restart()
        }
    }
    Timer { id: writeResultTimer; interval: 3000; onTriggered: writeResultLabel.visible = false }

    // =======================================================================
    //  LAYER 0 – BACKGROUND (gradient + particles + blurred album art)
    // =======================================================================
    Item {
        id: backgroundLayer
        anchors.fill: parent

        // Animated mesh gradient
        MeshGradientBackground {
            id: meshBg
            anchors.fill: parent
            color1: albumArtView.dominantColor1
            color2: albumArtView.dominantColor2
            color3: albumArtView.dominantColor3
            color4: albumArtView.dominantColor4
        }

        // Blurred album art environment reflection
        Image {
            id: bgAlbumArt
            anchors.fill: parent
            source: playerController.albumArtUrl
            fillMode: Image.PreserveAspectCrop
            visible: false
        }
        FastBlur {
            id: bgAlbumBlur
            anchors.fill: parent
            source: bgAlbumArt
            radius: 80
            opacity: playerController.albumArtUrl !== "" ? 0.18 : 0
            Behavior on opacity { NumberAnimation { duration: 800 } }
        }

        // Floating particles
        ParticleField {
            anchors.fill: parent
            particleColor: theme.accent
            particleCount: 30
            maxOpacity: 0.18
        }
    }

    // =======================================================================
    //  PARALLAX MOUSE TRACKER
    // =======================================================================
    MouseArea {
        id: parallaxTracker
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true
        onPositionChanged: function(mouse) {
            root.parallaxX = (mouse.x / root.width - 0.5) * 2
            root.parallaxY = (mouse.y / root.height - 0.5) * 2
        }
    }

    // =======================================================================
    //  LAYER 1 – MAIN CONTENT (inside glass panel with parallax offset)
    // =======================================================================
    Item {
        id: contentWrapper
        anchors.fill: parent
        anchors.margins: 12

        // Subtle parallax shift
        transform: Translate {
            x: root.parallaxX * 3
            y: root.parallaxY * 3
            Behavior on x { SpringAnimation { spring: 1.5; damping: 0.5 } }
            Behavior on y { SpringAnimation { spring: 1.5; damping: 0.5 } }
        }

        GlassPanel {
            id: mainPanel
            anchors.fill: parent
            backgroundSource: backgroundLayer
            blurRadius: theme.glassBlur
            tintColor: theme.glassTint
            borderOpacity: theme.glassBorder
            cornerRadius: theme.glassRadius

            // Breathing animation tied to playback
            scale: playerController.isPlaying ? breathAnim.value : 1.0
            transformOrigin: Item.Center
            QtObject {
                id: breathAnim
                property real value: 1.0
                NumberAnimation on value {
                    from: 1.0; to: 1.008; duration: 1200
                    easing.type: Easing.InOutSine
                    loops: Animation.Infinite
                    running: playerController.isPlaying
                }
            }

            Flickable {
                id: mainFlickable
                anchors.fill: parent
                anchors.margins: 16
                contentHeight: mainColumn.implicitHeight
                clip: true
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.DragOverBounds

                // Spring physics for overscroll
                rebound: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        duration: 600
                        easing.type: Easing.OutElastic
                        easing.amplitude: 1.0
                        easing.period: 0.4
                    }
                }

                ColumnLayout {
                    id: mainColumn
                    width: mainFlickable.width
                    spacing: 16

                    // -------------------------------------------------------
                    //  HEADER – language + theme toggles
                    // -------------------------------------------------------
                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.fillWidth: true }

                        GlassButton {
                            text: translationManager.language === "zh" ? "EN" : "中"
                            textColor: theme.text
                            borderColor: theme.border
                            baseColor: theme.buttonBg
                            hoverColor: theme.buttonHoverBg
                            cornerRadius: 10
                            Accessible.name: translationManager.language === "zh"
                                ? qsTr("Switch to English") : qsTr("Switch to Chinese")
                            onClicked: translationManager.setLanguage(
                                translationManager.language === "zh" ? "en" : "zh")
                        }

                        GlassButton {
                            iconText: root.darkTheme ? "☀" : "🌙"
                            text: ""
                            textColor: theme.text
                            borderColor: theme.border
                            baseColor: theme.buttonBg
                            hoverColor: theme.buttonHoverBg
                            cornerRadius: 10
                            Accessible.name: root.darkTheme ? qsTr("Switch to light theme") : qsTr("Switch to dark theme")
                            onClicked: root.darkTheme = !root.darkTheme
                        }
                    }

                    // -------------------------------------------------------
                    //  ALBUM ART – holographic 3D display
                    // -------------------------------------------------------
                    AlbumArtDisplay {
                        id: albumArtView
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: Math.min(mainFlickable.width * 0.55, 220)
                        Layout.preferredHeight: Layout.preferredWidth
                        source: playerController.albumArtUrl
                        isPlaying: playerController.isPlaying
                        parallaxX: root.parallaxX
                        parallaxY: root.parallaxY
                    }

                    // -------------------------------------------------------
                    //  TRACK INFO
                    // -------------------------------------------------------
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

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
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            font.family: "sans-serif"
                            color: theme.text

                            // Subtle glow on track title
                            layer.enabled: playerController.trackTitle !== ""
                            layer.effect: Glow {
                                radius: 4; samples: 9; spread: 0.05
                                color: Qt.rgba(theme.accent.r, theme.accent.g,
                                               theme.accent.b, 0.25)
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            visible: playerController.trackArtist !== ""
                            text: playerController.trackArtist
                                  + (playerController.trackAlbum !== ""
                                     ? " — " + playerController.trackAlbum : "")
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideMiddle
                            font.pixelSize: 13
                            font.family: "sans-serif"
                            color: theme.secondaryText
                        }

                        // Metadata action buttons
                        RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                            spacing: 8
                            visible: playerController.trackTitle !== ""

                            GlassButton {
                                text: qsTr("Choose Result...")
                                textColor: theme.text
                                borderColor: theme.border
                                baseColor: theme.buttonBg
                                hoverColor: theme.buttonHoverBg
                                visible: playerController.metadataResults.length > 1
                                onClicked: metadataSelectionDialogLoader.show()
                            }
                            GlassButton {
                                text: qsTr("Save to File")
                                textColor: theme.text
                                borderColor: theme.border
                                baseColor: theme.buttonBg
                                hoverColor: theme.buttonHoverBg
                                onClicked: {
                                    if (playerController.metadataWriteSupported)
                                        writeMetadataDialogLoader.show()
                                    else
                                        taglibMissingDialogLoader.show()
                                }
                            }
                        }

                        // Fingerprint indicator
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            visible: playerController.fingerprintAvailable && playerController.currentTrack !== ""
                            text: "♪ " + qsTr("Audio fingerprint identification active")
                            font.pixelSize: 10
                            font.family: "sans-serif"
                            color: theme.success
                        }

                        // Write result notification
                        Text {
                            id: writeResultLabel
                            Layout.alignment: Qt.AlignHCenter
                            visible: false
                            font.pixelSize: 11
                            font.family: "sans-serif"
                            color: theme.info
                        }
                    }

                    // -------------------------------------------------------
                    //  PROGRESS BAR – glowing flow line
                    // -------------------------------------------------------
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        GlowSlider {
                            id: progressSlider
                            Layout.fillWidth: true
                            from: 0
                            to: playerController.duration > 0 ? playerController.duration : 1
                            value: playerController.position
                            enabled: playerController.duration > 0
                            glowColor: theme.accent
                            trackColor: root.darkTheme ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)
                            onMoved: playerController.seek(value)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: formatTime(playerController.position)
                                font.pixelSize: 10; font.family: "sans-serif"
                                color: theme.mutedText
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: formatTime(playerController.duration)
                                font.pixelSize: 10; font.family: "sans-serif"
                                color: theme.mutedText
                            }
                        }
                    }

                    // -------------------------------------------------------
                    //  TRANSPORT CONTROLS – glass buttons with icons
                    // -------------------------------------------------------
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10

                        GlassButton {
                            iconText: "⏮"; text: ""
                            textColor: theme.text; borderColor: theme.border
                            baseColor: theme.buttonBg; hoverColor: theme.buttonHoverBg
                            cornerRadius: 20
                            Accessible.name: qsTr("Previous")
                            onClicked: playerController.previous()
                        }
                        GlassButton {
                            iconText: playerController.isPlaying ? "⏸" : "▶"
                            text: ""
                            isAccent: true
                            accentGlow: theme.accent
                            textColor: theme.text; borderColor: theme.border
                            baseColor: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.15)
                            hoverColor: Qt.rgba(theme.accent.r, theme.accent.g, theme.accent.b, 0.25)
                            cornerRadius: 24
                            leftPadding: 18; rightPadding: 18; topPadding: 12; bottomPadding: 12
                            Accessible.name: playerController.isPlaying ? qsTr("Pause") : qsTr("Play")
                            onClicked: {
                                if (playerController.isPlaying) playerController.pause()
                                else playerController.play()
                            }
                        }
                        GlassButton {
                            iconText: "⏹"; text: ""
                            textColor: theme.text; borderColor: theme.border
                            baseColor: theme.buttonBg; hoverColor: theme.buttonHoverBg
                            cornerRadius: 20
                            Accessible.name: qsTr("Stop")
                            onClicked: playerController.stop()
                        }
                        GlassButton {
                            iconText: "⏭"; text: ""
                            textColor: theme.text; borderColor: theme.border
                            baseColor: theme.buttonBg; hoverColor: theme.buttonHoverBg
                            cornerRadius: 20
                            Accessible.name: qsTr("Next")
                            onClicked: playerController.next()
                        }
                    }

                    // -------------------------------------------------------
                    //  VOLUME – glowing slider with label
                    // -------------------------------------------------------
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10

                        // Volume icon with brightness based on level
                        Text {
                            text: volumeSlider.value > 0.5 ? "🔊"
                                  : (volumeSlider.value > 0 ? "🔉" : "🔇")
                            font.pixelSize: 16
                            opacity: 0.4 + volumeSlider.value * 0.6
                            Behavior on opacity { NumberAnimation { duration: 200 } }
                        }

                        GlowSlider {
                            id: volumeSlider
                            from: 0.0; to: 1.0
                            value: playerController.volume
                            glowColor: theme.accent
                            trackColor: root.darkTheme ? Qt.rgba(1,1,1,0.08) : Qt.rgba(0,0,0,0.08)
                            implicitWidth: 160
                            onMoved: playerController.volume = value
                        }

                        Text {
                            text: Math.round(volumeSlider.value * 100) + "%"
                            font.pixelSize: 11; font.family: "sans-serif"
                            color: theme.mutedText
                            Layout.preferredWidth: 32
                        }
                    }

                    // -------------------------------------------------------
                    //  ADD FILES / FOLDER
                    // -------------------------------------------------------
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 8

                        GlassButton {
                            iconText: "＋"; text: qsTr("Files")
                            textColor: theme.text; borderColor: theme.border
                            baseColor: theme.buttonBg; hoverColor: theme.buttonHoverBg
                            onClicked: fileDialog.open()
                        }
                        GlassButton {
                            iconText: "📂"; text: qsTr("Folder")
                            textColor: theme.text; borderColor: theme.border
                            baseColor: theme.buttonBg; hoverColor: theme.buttonHoverBg
                            onClicked: folderDialog.open()
                        }
                    }

                    // -------------------------------------------------------
                    //  LYRICS PANEL – immersive depth lyrics
                    // -------------------------------------------------------
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Lyrics")
                                font.pixelSize: 13; font.weight: Font.DemiBold
                                font.family: "sans-serif"
                                color: theme.text
                            }
                            Item { Layout.fillWidth: true }
                            GlassButton {
                                iconText: lyricsPanel.visible ? "▲" : "▼"
                                text: ""
                                textColor: theme.mutedText
                                borderColor: "transparent"
                                baseColor: "transparent"
                                hoverColor: theme.hoverBg
                                cornerRadius: 8
                                leftPadding: 8; rightPadding: 8; topPadding: 4; bottomPadding: 4
                                Accessible.name: lyricsPanel.visible ? qsTr("Collapse lyrics") : qsTr("Expand lyrics")
                                onClicked: lyricsPanel.visible = !lyricsPanel.visible
                            }
                        }

                        ImmersiveLyrics {
                            id: lyricsPanel
                            Layout.fillWidth: true
                            Layout.preferredHeight: 140
                            visible: playerController.lyrics !== ""
                            lyrics: playerController.lyrics
                            textColor: theme.text
                            glowColor: theme.accent
                        }

                        Text {
                            visible: playerController.lyrics === "" && playerController.currentTrack !== ""
                            text: qsTr("(no lyrics available)")
                            color: theme.placeholderText
                            font.pixelSize: 11; font.family: "sans-serif"
                        }
                    }

                    // -------------------------------------------------------
                    //  PLAYLIST
                    // -------------------------------------------------------
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: qsTr("Playlist") + " (" + playerController.trackCount + ")"
                            font.pixelSize: 13; font.weight: Font.DemiBold
                            font.family: "sans-serif"
                            color: theme.text
                        }

                        // Playlist items (use Repeater inside Column since we're in a Flickable)
                        Column {
                            Layout.fillWidth: true
                            spacing: 2

                            Repeater {
                                model: playerController.trackList

                                Rectangle {
                                    width: parent.width
                                    height: 38
                                    radius: 8
                                    color: index === playerController.currentIndex
                                           ? theme.accentBg
                                           : (plMouseArea.containsMouse ? theme.hoverBg : "transparent")
                                    border.width: index === playerController.currentIndex ? 1 : 0
                                    border.color: Qt.rgba(theme.accent.r, theme.accent.g,
                                                          theme.accent.b, 0.3)

                                    Behavior on color {
                                        ColorAnimation { duration: 200; easing.type: Easing.OutCubic }
                                    }

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 10; anchors.rightMargin: 10
                                        spacing: 8

                                        Text {
                                            text: (index + 1) + "."
                                            Layout.preferredWidth: 28
                                            font.pixelSize: 11; font.family: "sans-serif"
                                            color: index === playerController.currentIndex
                                                   ? theme.accent : theme.mutedText
                                        }
                                        Text {
                                            text: modelData
                                            Layout.fillWidth: true
                                            elide: Text.ElideMiddle
                                            font.pixelSize: 12; font.family: "sans-serif"
                                            font.weight: index === playerController.currentIndex
                                                         ? Font.DemiBold : Font.Normal
                                            color: theme.text
                                        }
                                        GlassButton {
                                            iconText: "✕"; text: ""
                                            textColor: theme.mutedText
                                            borderColor: "transparent"
                                            baseColor: "transparent"
                                            hoverColor: Qt.rgba(1, 0.3, 0.3, 0.15)
                                            cornerRadius: 6
                                            leftPadding: 6; rightPadding: 6; topPadding: 2; bottomPadding: 2
                                            Accessible.name: qsTr("Remove")
                                            onClicked: playerController.removeTrack(index)
                                        }
                                    }

                                    MouseArea {
                                        id: plMouseArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        acceptedButtons: Qt.LeftButton
                                        onDoubleClicked: playerController.selectTrack(index)
                                        z: -1
                                    }
                                }
                            }

                            // Empty playlist placeholder
                            Item {
                                width: parent.width
                                height: 60
                                visible: playerController.trackCount === 0
                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("(playlist is empty)")
                                    color: theme.placeholderText
                                    font.pixelSize: 12; font.family: "sans-serif"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
