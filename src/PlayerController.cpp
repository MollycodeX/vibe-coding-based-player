// PlayerController.cpp
// Bridges the Qt QML front-end to the FFmpeg/miniaudio-backed AudioPlayer/Playlist.

#include "PlayerController.h"
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QFile>

static const QStringList audioExtensions = {
    "mp3", "wav", "flac", "ogg", "aac", "wma", "m4a", "opus"
};

// Extracts a human-readable track name from a file path by stripping the
// directory prefix and the file extension.
static QString trackNameFromPath(const QString &path)
{
    QString name = QFileInfo(path).completeBaseName();
    // Many files use "Artist - Title" convention; return as-is so the
    // metadata provider can search for it.
    return name;
}

PlayerController::PlayerController(QObject *parent)
    : QObject(parent)
{
    // Metadata provider signals
    connect(&m_metadataProvider, &MetadataProvider::metadataReady, this,
            &PlayerController::onMetadataReady);
    connect(&m_metadataProvider, &MetadataProvider::multipleResultsReady, this,
            &PlayerController::onMultipleResultsReady);

    // Lyrics provider
    connect(&m_lyricsProvider, &LyricsProvider::lyricsReady, this,
            &PlayerController::onLyricsReady);

    // Audio fingerprinter
    connect(&m_fingerprinter, &AudioFingerprinter::fingerprintReady, this,
            &PlayerController::onFingerprintReady);
    connect(&m_fingerprinter, &AudioFingerprinter::fingerprintFailed, this,
            &PlayerController::onFingerprintFailed);

    // AcoustID client
    connect(&m_acoustIdClient, &AcoustIdClient::resultsReady, this,
            &PlayerController::onAcoustIdResultsReady);
    connect(&m_acoustIdClient, &AcoustIdClient::lookupFailed, this,
            &PlayerController::onAcoustIdFailed);

    // Cover art provider
    connect(&m_coverArtProvider, &CoverArtProvider::coverArtReady, this,
            &PlayerController::onCoverArtReady);
    connect(&m_coverArtProvider, &CoverArtProvider::coverArtFailed, this,
            &PlayerController::onCoverArtFailed);
}

PlayerController::~PlayerController()
{
    // Clean up the temporary cover art file on destruction.
    if (!m_albumArtUrl.isEmpty()) {
        QUrl url(m_albumArtUrl);
        if (url.isLocalFile())
            QFile::remove(url.toLocalFile());
    }
}

bool PlayerController::isPlaying() const
{
    return m_player.isPlaying();
}

float PlayerController::volume() const
{
    return m_player.getVolume();
}

void PlayerController::setVolume(float v)
{
    m_player.setVolume(v);
    emit volumeChanged();
}

QString PlayerController::currentTrack() const
{
    return QString::fromUtf8(m_player.getCurrentTrack().c_str());
}

QStringList PlayerController::trackList() const
{
    const auto &tracks = m_playlist.getTracks();
    QStringList list;
    list.reserve(static_cast<int>(tracks.size()));
    for (const auto &t : tracks)
        list.append(QString::fromUtf8(t.c_str()));
    return list;
}

int PlayerController::trackCount() const
{
    return m_playlist.trackCount();
}

int PlayerController::currentIndex() const
{
    return m_playlist.currentIndex();
}

float PlayerController::position() const
{
    return m_player.getPositionSeconds();
}

float PlayerController::duration() const
{
    return m_player.getDurationSeconds();
}

QString PlayerController::trackTitle() const { return m_trackTitle; }
QString PlayerController::trackArtist() const { return m_trackArtist; }
QString PlayerController::trackAlbum() const { return m_trackAlbum; }
QString PlayerController::lyrics() const { return m_lyrics; }

QVariantList PlayerController::metadataResults() const { return m_metadataResults; }

QString PlayerController::albumArtUrl() const { return m_albumArtUrl; }

bool PlayerController::embedAlbumArt() const { return m_embedAlbumArt; }

void PlayerController::setEmbedAlbumArt(bool embed)
{
    if (m_embedAlbumArt != embed) {
        m_embedAlbumArt = embed;
        emit embedAlbumArtChanged();
    }
}

bool PlayerController::fingerprintAvailable() const
{
    return AudioFingerprinter::isAvailable();
}

bool PlayerController::metadataWriteSupported() const
{
    return MetadataWriter::isSupported();
}

void PlayerController::play()
{
    if (!m_player.getCurrentTrack().empty()) {
        m_player.play();
    } else if (m_playlist.hasTrack()) {
        m_player.loadTrack(m_playlist.currentTrack());
        m_player.play();
        emit currentTrackChanged();
        emit durationChanged();
        lookupTrackInfo();
    }
    emit playingChanged();
}

void PlayerController::pause()
{
    m_player.pause();
    emit playingChanged();
}

void PlayerController::stop()
{
    m_player.stop();
    emit playingChanged();
    emit positionChanged();
}

void PlayerController::next()
{
    if (m_playlist.hasTrack()) {
        const std::string &track = m_playlist.nextTrack();
        m_player.loadTrack(track);
        m_player.play();
        emit currentTrackChanged();
        emit playingChanged();
        emit playlistChanged();
        emit durationChanged();
        lookupTrackInfo();
    }
}

void PlayerController::previous()
{
    if (m_playlist.hasTrack()) {
        const std::string &track = m_playlist.previousTrack();
        m_player.loadTrack(track);
        m_player.play();
        emit currentTrackChanged();
        emit playingChanged();
        emit playlistChanged();
        emit durationChanged();
        lookupTrackInfo();
    }
}

void PlayerController::addTrack(const QString &filePath)
{
    // Explicitly convert via UTF-8 so that CJK and other non-ASCII characters
    // in file paths are preserved correctly.
    m_playlist.addTrack(filePath.toUtf8().constData());
    emit playlistChanged();
    // Auto-load the first added track so currentTrack shows up immediately.
    if (m_playlist.trackCount() == 1) {
        m_player.loadTrack(m_playlist.currentTrack());
        emit currentTrackChanged();
        emit durationChanged();
        lookupTrackInfo();
    }
}

void PlayerController::addFolder(const QString &folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) return;

    QStringList filters;
    for (const QString &ext : audioExtensions)
        filters << ("*." + ext);

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &fi : files)
        addTrack(fi.absoluteFilePath());
}

void PlayerController::addTrackUrl(const QUrl &fileUrl)
{
    addTrack(fileUrl.toLocalFile());
}

void PlayerController::addFolderUrl(const QUrl &folderUrl)
{
    addFolder(folderUrl.toLocalFile());
}

void PlayerController::removeTrack(int index)
{
    if (index < 0 || index >= m_playlist.trackCount()) return;

    bool removingCurrent = (index == m_playlist.currentIndex());
    m_playlist.removeTrack(index);
    emit playlistChanged();

    if (removingCurrent) {
        m_player.stop();
        if (m_playlist.hasTrack()) {
            m_player.loadTrack(m_playlist.currentTrack());
            lookupTrackInfo();
        } else {
            clearMetadata();
        }
        emit currentTrackChanged();
        emit playingChanged();
        emit durationChanged();
    }
}

void PlayerController::selectTrack(int index)
{
    if (!m_playlist.setCurrentIndex(index)) return;

    m_player.loadTrack(m_playlist.currentTrack());
    m_player.play();
    emit currentTrackChanged();
    emit playingChanged();
    emit playlistChanged();
    emit durationChanged();
    lookupTrackInfo();
}

void PlayerController::seek(float seconds)
{
    m_player.seekTo(seconds);
    emit positionChanged();
}

void PlayerController::updatePosition()
{
    emit positionChanged();
}

// ---------------------------------------------------------------------------
// Metadata lookup orchestration
// ---------------------------------------------------------------------------

void PlayerController::lookupTrackInfo()
{
    QString path = currentTrack();
    if (path.isEmpty())
        return;

    clearMetadata();

    // Strategy: try audio fingerprinting first (works even without metadata).
    // If fpcalc is available, compute fingerprint → AcoustID → results.
    // Otherwise, fall back to filename-based MusicBrainz search.
    if (AudioFingerprinter::isAvailable()) {
        m_fingerprinter.compute(path);
    } else {
        // Fallback: search by filename
        QString name = trackNameFromPath(path);
        m_metadataProvider.lookup(name);
    }
}

void PlayerController::clearMetadata()
{
    m_trackTitle.clear();
    m_trackArtist.clear();
    m_trackAlbum.clear();
    m_lyrics.clear();
    m_metadataResults.clear();
    m_albumArtUrl.clear();
    m_albumArtData.clear();
    emit metadataChanged();
    emit lyricsChanged();
    emit metadataResultsChanged();
    emit albumArtChanged();
}

void PlayerController::onFingerprintReady(int duration, const QString &fingerprint)
{
    // Send fingerprint to AcoustID for identification.
    m_acoustIdClient.lookup(duration, fingerprint);
}

void PlayerController::onFingerprintFailed(const QString & /*errorMessage*/)
{
    // Fallback to filename-based MusicBrainz search.
    QString path = currentTrack();
    if (!path.isEmpty()) {
        QString name = trackNameFromPath(path);
        m_metadataProvider.lookup(name);
    }
}

void PlayerController::onAcoustIdResultsReady(const QVariantList &results)
{
    // AcoustID returned results – expose them for user selection.
    m_metadataResults = results;
    emit metadataResultsChanged();

    // Auto-select the first (highest-scoring) result.
    if (!results.isEmpty()) {
        selectMetadataResult(0);
    }
}

void PlayerController::onAcoustIdFailed(const QString & /*errorMessage*/)
{
    // AcoustID failed – fall back to filename-based search.
    QString path = currentTrack();
    if (!path.isEmpty()) {
        QString name = trackNameFromPath(path);
        m_metadataProvider.lookup(name);
    }
}

void PlayerController::onMultipleResultsReady(const QVariantList &results)
{
    m_metadataResults = results;
    emit metadataResultsChanged();
}

void PlayerController::onMetadataReady(const QString &artist, const QString &album,
                                       const QString &title)
{
    m_trackTitle = title;
    m_trackArtist = artist;
    m_trackAlbum = album;
    emit metadataChanged();

    // Now that we have structured metadata, look up lyrics with artist info.
    m_lyricsProvider.lookup(title, artist);
}

void PlayerController::onLyricsReady(const QString &plainLyrics,
                                     const QString & /*syncedLyrics*/)
{
    m_lyrics = plainLyrics;
    emit lyricsChanged();
}

void PlayerController::selectMetadataResult(int index)
{
    if (index < 0 || index >= m_metadataResults.size())
        return;

    QVariantMap selected = m_metadataResults.at(index).toMap();
    m_trackTitle = selected.value(QStringLiteral("title")).toString();
    m_trackArtist = selected.value(QStringLiteral("artist")).toString();
    m_trackAlbum = selected.value(QStringLiteral("album")).toString();
    emit metadataChanged();

    // Fetch album cover art.
    fetchCoverArt(selected);

    // Look up lyrics using the selected metadata.
    if (!m_trackTitle.isEmpty())
        m_lyricsProvider.lookup(m_trackTitle, m_trackArtist);
}

void PlayerController::writeMetadataToFile()
{
    QString path = currentTrack();
    if (path.isEmpty() || m_trackTitle.isEmpty()) {
        emit metadataWritten(false);
        return;
    }

    QByteArray coverData;
    if (m_embedAlbumArt && !m_albumArtData.isEmpty())
        coverData = m_albumArtData;

    bool ok = m_metadataWriter.write(path, m_trackTitle, m_trackArtist,
                                     m_trackAlbum, coverData);
    emit metadataWritten(ok);
}

// ---------------------------------------------------------------------------
// Cover art helpers
// ---------------------------------------------------------------------------

void PlayerController::fetchCoverArt(const QVariantMap &selected)
{
    // Clear previous cover art.
    m_albumArtUrl.clear();
    m_albumArtData.clear();
    emit albumArtChanged();

    // Try releaseId first (from MusicBrainz), then releaseGroupId (from AcoustID).
    QString releaseId = selected.value(QStringLiteral("releaseId")).toString();
    if (!releaseId.isEmpty()) {
        m_coverArtProvider.fetchByReleaseId(releaseId);
        return;
    }
    QString releaseGroupId =
        selected.value(QStringLiteral("releaseGroupId")).toString();
    if (!releaseGroupId.isEmpty()) {
        m_coverArtProvider.fetchByReleaseGroupId(releaseGroupId);
    }
}

void PlayerController::onCoverArtReady(const QByteArray &imageData)
{
    m_albumArtData = imageData;

    // Save to a temporary file so the QML Image component can display it.
    QString tmpDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    // Use a unique filename to avoid conflicts between instances.
    QString tmpPath = tmpDir + QStringLiteral("/mscplayer_cover_%1.jpg")
                                   .arg(reinterpret_cast<quintptr>(this));

    // Remove the old temporary file before writing the new one.
    QFile::remove(tmpPath);

    QFile file(tmpPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        m_albumArtUrl = QUrl::fromLocalFile(tmpPath).toString();
    }

    emit albumArtChanged();
}

void PlayerController::onCoverArtFailed(const QString & /*errorMessage*/)
{
    // Cover art not available; leave the URL empty.
}
