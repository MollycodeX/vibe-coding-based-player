#include "player.hpp"
#include "Playlist.h"
#include <cassert>
#include <iostream>

// ---------------------------------------------------------------------------
// Playlist unit tests
// ---------------------------------------------------------------------------
static void testPlaylistBasic() {
    Playlist pl;
    assert(!pl.hasTrack());
    assert(pl.trackCount() == 0);
    assert(pl.currentIndex() == -1);
    assert(pl.currentTrack().empty());
    std::cout << "  [PASS] empty playlist\n";
}

static void testPlaylistAddAndCurrent() {
    Playlist pl;
    pl.addTrack("a.mp3");
    assert(pl.hasTrack());
    assert(pl.trackCount() == 1);
    assert(pl.currentIndex() == 0);
    assert(pl.currentTrack() == "a.mp3");

    pl.addTrack("b.mp3");
    assert(pl.trackCount() == 2);
    // Adding a second track must NOT change the cursor.
    assert(pl.currentIndex() == 0);
    assert(pl.currentTrack() == "a.mp3");
    std::cout << "  [PASS] addTrack / currentTrack\n";
}

static void testPlaylistNextPrev() {
    Playlist pl;
    pl.addTrack("a.mp3");
    pl.addTrack("b.mp3");
    pl.addTrack("c.mp3");

    assert(pl.nextTrack() == "b.mp3");
    assert(pl.currentIndex() == 1);

    assert(pl.nextTrack() == "c.mp3");
    assert(pl.currentIndex() == 2);

    // Wrap-around forward
    assert(pl.nextTrack() == "a.mp3");
    assert(pl.currentIndex() == 0);

    // Wrap-around backward
    assert(pl.previousTrack() == "c.mp3");
    assert(pl.currentIndex() == 2);

    assert(pl.previousTrack() == "b.mp3");
    assert(pl.currentIndex() == 1);
    std::cout << "  [PASS] nextTrack / previousTrack with wrap-around\n";
}

static void testPlaylistRemove() {
    Playlist pl;
    pl.addTrack("a.mp3");
    pl.addTrack("b.mp3");
    pl.addTrack("c.mp3");

    pl.removeTrack(1); // remove "b.mp3"
    assert(pl.trackCount() == 2);
    assert(pl.getTracks()[0] == "a.mp3");
    assert(pl.getTracks()[1] == "c.mp3");
    std::cout << "  [PASS] removeTrack\n";
}

static void testPlaylistClear() {
    Playlist pl;
    pl.addTrack("a.mp3");
    pl.addTrack("b.mp3");
    pl.clear();
    assert(!pl.hasTrack());
    assert(pl.trackCount() == 0);
    assert(pl.currentIndex() == -1);
    std::cout << "  [PASS] clear\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    // Keep the original Player smoke-test.
    Player p;
    p.play();

    std::cout << "\nRunning Playlist tests...\n";
    testPlaylistBasic();
    testPlaylistAddAndCurrent();
    testPlaylistNextPrev();
    testPlaylistRemove();
    testPlaylistClear();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
