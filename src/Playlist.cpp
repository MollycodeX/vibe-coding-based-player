// Playlist.cpp

#include "Playlist.h"
#include <algorithm>
#include <random>

const std::string Playlist::EMPTY_TRACK = "";

Playlist::Playlist() : currentIdx(-1), m_shufflePos(-1), m_mode(PlaybackMode::LoopAll), m_shuffleEnabled(false) {}

void Playlist::addTrack(const std::string& filePath) {
    tracks.push_back(filePath);
    if (currentIdx < 0) {
        currentIdx = 0;
    }
}

void Playlist::addTrack(std::string&& filePath) {
    tracks.push_back(std::move(filePath));
    if (currentIdx < 0) {
        currentIdx = 0;
    }
}

void Playlist::removeTrack(int index) {
    if (index < 0 || index >= static_cast<int>(tracks.size())) return;
    tracks.erase(tracks.begin() + index);
    if (tracks.empty()) {
        currentIdx = -1;
    } else if (currentIdx >= static_cast<int>(tracks.size())) {
        currentIdx = static_cast<int>(tracks.size()) - 1;
    }
}

void Playlist::clear() {
    tracks.clear();
    currentIdx = -1;
}

bool Playlist::hasTrack() const {
    return !tracks.empty();
}

int Playlist::trackCount() const {
    return static_cast<int>(tracks.size());
}

int Playlist::currentIndex() const {
    return currentIdx;
}

const std::string& Playlist::currentTrack() const {
    if (currentIdx < 0 || currentIdx >= static_cast<int>(tracks.size())) {
        return EMPTY_TRACK;
    }
    return tracks[currentIdx];
}

const std::string& Playlist::nextTrack() {
    if (tracks.empty()) return EMPTY_TRACK;
    
    if (m_mode == PlaybackMode::LoopOne) {
        return tracks[currentIdx];
    }

    if (m_shuffleEnabled) {
        if (m_shuffledIndices.size() != tracks.size()) {
            reshuffle();
        }
        m_shufflePos++;
        if (m_shufflePos >= static_cast<int>(m_shuffledIndices.size())) {
            if (m_mode == PlaybackMode::Sequential) {
                return EMPTY_TRACK;
            } else { // LoopAll
                reshuffle();
                m_shufflePos = 0;
            }
        }
        currentIdx = m_shuffledIndices[m_shufflePos];
        return tracks[currentIdx];
    } else {
        if (m_mode == PlaybackMode::Sequential) {
            if (currentIdx >= static_cast<int>(tracks.size()) - 1) {
                return EMPTY_TRACK;
            }
            currentIdx++;
            return tracks[currentIdx];
        } else { // LoopAll
            currentIdx = (currentIdx + 1) % static_cast<int>(tracks.size());
            return tracks[currentIdx];
        }
    }
}

const std::string& Playlist::previousTrack() {
    if (tracks.empty()) return EMPTY_TRACK;

    if (m_mode == PlaybackMode::LoopOne) {
        return tracks[currentIdx];
    }

    if (m_shuffleEnabled) {
        if (m_shuffledIndices.size() != tracks.size()) {
            reshuffle();
            m_shufflePos = 0;
        }
        m_shufflePos--;
        if (m_shufflePos < 0) {
            if (m_mode == PlaybackMode::Sequential) {
                m_shufflePos = 0; // Stick to the first song naturally
            } else {
                m_shufflePos = static_cast<int>(m_shuffledIndices.size()) - 1;
            }
        }
        currentIdx = m_shuffledIndices[m_shufflePos];
        return tracks[currentIdx];
    } else {
        if (m_mode == PlaybackMode::Sequential) {
            if (currentIdx <= 0) {
                currentIdx = 0;
                return tracks[currentIdx];
            }
            currentIdx--;
            return tracks[currentIdx];
        } else { // LoopAll
            currentIdx = (currentIdx - 1 + static_cast<int>(tracks.size())) % static_cast<int>(tracks.size());
            return tracks[currentIdx];
        }
    }
}

void Playlist::setPlaybackMode(PlaybackMode mode) {
    m_mode = mode;
}

Playlist::PlaybackMode Playlist::getPlaybackMode() const {
    return m_mode;
}

bool Playlist::isShuffleEnabled() const {
    return m_shuffleEnabled;
}

void Playlist::setShuffleEnabled(bool enabled) {
    if (m_shuffleEnabled == enabled) return;
    
    m_shuffleEnabled = enabled;
    if (m_shuffleEnabled) {
        reshuffle();
        // Sync shuffle position to keep playing currently audible track naturally progressing.
        for (int i = 0; i < static_cast<int>(m_shuffledIndices.size()); ++i) {
            if (m_shuffledIndices[i] == currentIdx) {
                m_shufflePos = i;
                break;
            }
        }
    }
}

void Playlist::reshuffle() {
    m_shuffledIndices.clear();
    for (int i = 0; i < static_cast<int>(tracks.size()); ++i) {
        m_shuffledIndices.push_back(i);
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_shuffledIndices.begin(), m_shuffledIndices.end(), g);
    m_shufflePos = -1;
}

const std::vector<std::string>& Playlist::getTracks() const {
    return tracks;
}

bool Playlist::setCurrentIndex(int index) {
    if (index < 0 || index >= static_cast<int>(tracks.size())) return false;
    currentIdx = index;
    return true;
}
