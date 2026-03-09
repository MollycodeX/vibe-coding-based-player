#include "player.hpp"
#include "Playlist.h"
#include "AudioDecoder.h"
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

static void testPlaylistSetCurrentIndex() {
    Playlist pl;
    pl.addTrack("a.mp3");
    pl.addTrack("b.mp3");
    pl.addTrack("c.mp3");

    assert(pl.setCurrentIndex(2));
    assert(pl.currentIndex() == 2);
    assert(pl.currentTrack() == "c.mp3");

    assert(pl.setCurrentIndex(0));
    assert(pl.currentIndex() == 0);
    assert(pl.currentTrack() == "a.mp3");

    // Out-of-range indices should fail and leave the cursor unchanged.
    assert(!pl.setCurrentIndex(-1));
    assert(pl.currentIndex() == 0);
    assert(!pl.setCurrentIndex(3));
    assert(pl.currentIndex() == 0);
    std::cout << "  [PASS] setCurrentIndex\n";
}

static void testPlaylistUnicodePaths() {
    Playlist pl;
    // Chinese characters in file path
    std::string chinesePath = "/music/\xe4\xb8\xad\xe6\x96\x87\xe6\xad\x8c\xe6\x9b\xb2.mp3";  // /music/中文歌曲.mp3
    // Japanese characters in file path
    std::string japanesePath = "/music/\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e.flac";  // /music/日本語.flac
    // Mixed ASCII and CJK
    std::string mixedPath = "/home/user/\xe9\x9f\xb3\xe6\xa5\xbd/track01.wav";  // /home/user/音楽/track01.wav

    pl.addTrack(chinesePath);
    pl.addTrack(japanesePath);
    pl.addTrack(mixedPath);

    assert(pl.trackCount() == 3);
    assert(pl.currentTrack() == chinesePath);
    assert(pl.nextTrack() == japanesePath);
    assert(pl.nextTrack() == mixedPath);

    // Verify round-trip: stored paths must be byte-identical to what was added.
    const auto& tracks = pl.getTracks();
    assert(tracks[0] == chinesePath);
    assert(tracks[1] == japanesePath);
    assert(tracks[2] == mixedPath);

    std::cout << "  [PASS] Unicode (CJK) file paths\n";
}

// ---------------------------------------------------------------------------
// AudioDecoder unit tests
// ---------------------------------------------------------------------------
static void testAudioDecoderInit() {
    AudioDecoder dec;
    // Initializing with a valid path should succeed.
    assert(dec.initialize("test.wav"));
    // Initializing with nullptr should fail.
    AudioDecoder nullPathDecoder;
    assert(!nullPathDecoder.initialize(nullptr));
    std::cout << "  [PASS] AudioDecoder initialize\n";
}

static void testAudioDecoderDecodeStub() {
    AudioDecoder dec;
    uint8_t* buf = nullptr;
    int size = 0;
    // Decoding without initialization should return -1.
    assert(dec.decode(&buf, &size) == -1);
    // Even after initialization the stub returns -1 (not yet implemented).
    dec.initialize("test.wav");
    assert(dec.decode(&buf, &size) == -1);
    std::cout << "  [PASS] AudioDecoder decode stub\n";
}

static void testAudioDecoderCleanup() {
    AudioDecoder dec;
    dec.initialize("test.wav");
    dec.cleanup();
    // After cleanup, decode should fail.
    uint8_t* buf = nullptr;
    int size = 0;
    assert(dec.decode(&buf, &size) == -1);
    std::cout << "  [PASS] AudioDecoder cleanup\n";
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
    testPlaylistSetCurrentIndex();
    testPlaylistUnicodePaths();

    std::cout << "\nRunning AudioDecoder tests...\n";
    testAudioDecoderInit();
    testAudioDecoderDecodeStub();
    testAudioDecoderCleanup();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
