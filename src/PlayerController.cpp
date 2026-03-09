// PlayerController.cpp
// Bridges the Qt QML front-end to the miniaudio-backed AudioPlayer/Playlist.

#include "PlayerController.h"
#include <QFileInfo>

static const QStringList audioExtensions = {
    "mp3", "wav", "flac", "ogg", "aac", "wma", "m4a", "opus"
};

PlayerController::PlayerController(QObject *parent)
    : QObject(parent)
{
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
    return QString::fromStdString(m_player.getCurrentTrack());
}

QStringList PlayerController::trackList() const
{
    QStringList list;
    for (const auto &t : m_playlist.getTracks())
        list.append(QString::fromStdString(t));
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

void PlayerController::play()
{
    if (!m_player.getCurrentTrack().empty()) {
        m_player.play();
    } else if (m_playlist.hasTrack()) {
        m_player.loadTrack(m_playlist.currentTrack());
        m_player.play();
        emit currentTrackChanged();
        emit durationChanged();
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
    }
}

void PlayerController::addTrack(const QString &filePath)
{
    m_playlist.addTrack(filePath.toStdString());
    emit playlistChanged();
    // Auto-load the first added track so currentTrack shows up immediately.
    if (m_playlist.trackCount() == 1) {
        m_player.loadTrack(m_playlist.currentTrack());
        emit currentTrackChanged();
        emit durationChanged();
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
