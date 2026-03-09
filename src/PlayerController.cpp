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
    }
}

void PlayerController::addTrack(const QString &filePath)
{
    m_playlist.addTrack(filePath.toStdString());
    // Auto-load the first added track so currentTrack shows up immediately.
    if (m_playlist.trackCount() == 1) {
        m_player.loadTrack(m_playlist.currentTrack());
        emit currentTrackChanged();
    }
}
