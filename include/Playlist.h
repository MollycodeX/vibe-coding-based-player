// Playlist.h
// Manages an ordered list of audio file paths with a current-track cursor.

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <vector>
#include <string>

class Playlist {
public:
    Playlist();

    // Add a file path to the end of the playlist.
    void addTrack(const std::string& filePath);

    // Remove the track at the given index.
    void removeTrack(int index);

    // Remove all tracks and reset the cursor.
    void clear();

    bool hasTrack()    const;
    int  trackCount()  const;
    int  currentIndex() const;

    // Returns the current track path, or "" if the playlist is empty.
    const std::string& currentTrack() const;

    // Advance to the next track (wraps around) and return its path.
    const std::string& nextTrack();

    // Go back to the previous track (wraps around) and return its path.
    const std::string& previousTrack();

    const std::vector<std::string>& getTracks() const;

private:
    std::vector<std::string> tracks;
    int                      currentIdx;

    static const std::string EMPTY_TRACK;
};

#endif // PLAYLIST_H
