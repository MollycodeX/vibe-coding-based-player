// PlayerController.cpp
// Bridges the Qt QML front-end to the miniaudio-backed AudioPlayer/Playlist.

#include "PlayerController.h"

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

void PlayerController::play()
{
    if (!m_player.getCurrentTrack().empty()) {
        m_player.play();
    } else if (m_playlist.hasTrack()) {
        m_player.loadTrack(m_playlist.currentTrack());
        m_player.play();
        emit currentTrackChanged();
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
    }
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
    }
}

void PlayerController::selectTrack(int index)
{
    const auto &tracks = m_playlist.getTracks();
    if (index < 0 || index >= static_cast<int>(tracks.size())) return;

    // Navigate to the chosen track by adjusting the internal cursor.
    while (m_playlist.currentIndex() != index) {
        m_playlist.nextTrack();
    }

    m_player.loadTrack(m_playlist.currentTrack());
    m_player.play();
    emit currentTrackChanged();
    emit playingChanged();
    emit playlistChanged();
}
