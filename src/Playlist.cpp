// Playlist.cpp

#include "Playlist.h"

const std::string Playlist::EMPTY_TRACK = "";

Playlist::Playlist() : currentIdx(-1) {}

void Playlist::addTrack(const std::string& filePath) {
    tracks.push_back(filePath);
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
    currentIdx = (currentIdx + 1) % static_cast<int>(tracks.size());
    return tracks[currentIdx];
}

const std::string& Playlist::previousTrack() {
    if (tracks.empty()) return EMPTY_TRACK;
    currentIdx = (currentIdx - 1 + static_cast<int>(tracks.size()))
                 % static_cast<int>(tracks.size());
    return tracks[currentIdx];
}

const std::vector<std::string>& Playlist::getTracks() const {
    return tracks;
}

bool Playlist::setCurrentIndex(int index) {
    if (index < 0 || index >= static_cast<int>(tracks.size())) return false;
    currentIdx = index;
    return true;
}
