// Playlist.h

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <vector>
#include <string>

class Playlist {
public:
    Playlist();
    ~Playlist();

    void addSong(const std::string& song);
    void removeSong(const std::string& song);
    void clear();
    const std::vector<std::string>& getSongs() const;

private:
    std::vector<std::string> songs;
};

#endif // PLAYLIST_H
