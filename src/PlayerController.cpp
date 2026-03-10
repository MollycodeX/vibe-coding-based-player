// PlayerController.cpp
// Bridges the Qt QML front-end to the miniaudio-backed AudioPlayer/Playlist.

#include "PlayerController.h"
#include <QFileInfo>
#include <QUrl>

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
    connect(&m_metadataProvider, &MetadataProvider::metadataReady, this,
            &PlayerController::onMetadataReady);
    connect(&m_lyricsProvider, &LyricsProvider::lyricsReady, this,
            &PlayerController::onLyricsReady);
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
    QStringList list;
    for (const auto &t : m_playlist.getTracks())
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
            // Clear metadata and lyrics when playlist becomes empty.
            m_trackTitle.clear();
            m_trackArtist.clear();
            m_trackAlbum.clear();
            m_lyrics.clear();
            emit metadataChanged();
            emit lyricsChanged();
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

void PlayerController::lookupTrackInfo()
{
    QString path = currentTrack();
    if (path.isEmpty())
        return;

    // Reset current metadata / lyrics while looking up.
    m_trackTitle.clear();
    m_trackArtist.clear();
    m_trackAlbum.clear();
    m_lyrics.clear();
    emit metadataChanged();
    emit lyricsChanged();

    QString name = trackNameFromPath(path);
    m_metadataProvider.lookup(name);
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
