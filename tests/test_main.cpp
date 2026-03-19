#include "Playlist.h"
#include "MetadataProvider.h"
#include "LyricsProvider.h"
#include "AudioFingerprinter.h"
#include "AcoustIdClient.h"
#include "MetadataWriter.h"
#include "CoverArtProvider.h"
#include <QUrl>
#include <QUrlQuery>
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
// MetadataProvider unit tests (URL building & JSON parsing – no network)
// ---------------------------------------------------------------------------
static void testMetadataProviderBuildUrl() {
    QUrl url = MetadataProvider::buildSearchUrl("Bohemian Rhapsody");
    assert(url.host() == "musicbrainz.org");
    assert(url.path() == "/ws/2/recording");
    QUrlQuery q(url);
    assert(q.hasQueryItem("query"));
    assert(q.queryItemValue("query") == "Bohemian Rhapsody");
    assert(q.queryItemValue("fmt") == "json");
    assert(q.queryItemValue("limit") == "20");
    std::cout << "  [PASS] MetadataProvider::buildSearchUrl\n";
}

static void testMetadataProviderBuildUrlWithLimit() {
    QUrl url = MetadataProvider::buildSearchUrl("Test", 5);
    QUrlQuery q(url);
    assert(q.queryItemValue("limit") == "5");
    std::cout << "  [PASS] MetadataProvider::buildSearchUrl (custom limit)\n";
}

static void testMetadataProviderParseResponse() {
    // Minimal MusicBrainz-like JSON
    QByteArray json = R"({
        "recordings": [{
            "title": "Bohemian Rhapsody",
            "artist-credit": [{"artist": {"name": "Queen"}}],
            "releases": [{"title": "A Night at the Opera"}]
        }]
    })";
    QString artist, album, title;
    assert(MetadataProvider::parseResponse(json, artist, album, title));
    assert(title == "Bohemian Rhapsody");
    assert(artist == "Queen");
    assert(album == "A Night at the Opera");
    std::cout << "  [PASS] MetadataProvider::parseResponse (valid)\n";
}

static void testMetadataProviderParseEmpty() {
    // Empty recordings array
    QByteArray json = R"({"recordings": []})";
    QString artist, album, title;
    assert(!MetadataProvider::parseResponse(json, artist, album, title));

    // Invalid JSON
    QByteArray badJson = "not json";
    assert(!MetadataProvider::parseResponse(badJson, artist, album, title));
    std::cout << "  [PASS] MetadataProvider::parseResponse (empty/invalid)\n";
}

// ---------------------------------------------------------------------------
// LyricsProvider unit tests (URL building & JSON parsing – no network)
// ---------------------------------------------------------------------------
static void testLyricsProviderBuildUrl() {
    QUrl url = LyricsProvider::buildSearchUrl("Bohemian Rhapsody", "Queen");
    assert(url.host() == "lrclib.net");
    assert(url.path() == "/api/search");
    QUrlQuery q(url);
    assert(q.queryItemValue("track_name") == "Bohemian Rhapsody");
    assert(q.queryItemValue("artist_name") == "Queen");
    std::cout << "  [PASS] LyricsProvider::buildSearchUrl\n";
}

static void testLyricsProviderBuildUrlNoArtist() {
    QUrl url = LyricsProvider::buildSearchUrl("Bohemian Rhapsody", "");
    QUrlQuery q(url);
    assert(q.hasQueryItem("track_name"));
    assert(!q.hasQueryItem("artist_name"));
    std::cout << "  [PASS] LyricsProvider::buildSearchUrl (no artist)\n";
}

static void testLyricsProviderParseResponse() {
    // LRCLIB-like JSON array
    QByteArray json = R"([{
        "plainLyrics": "Is this the real life?\nIs this just fantasy?",
        "syncedLyrics": "[00:00.50] Is this the real life?\n[00:03.20] Is this just fantasy?"
    }])";
    QString plain, synced;
    assert(LyricsProvider::parseResponse(json, plain, synced));
    assert(plain.contains("Is this the real life?"));
    assert(synced.contains("[00:00.50]"));
    std::cout << "  [PASS] LyricsProvider::parseResponse (valid)\n";
}

static void testLyricsProviderParseEmpty() {
    // Empty array
    QByteArray json = "[]";
    QString plain, synced;
    assert(!LyricsProvider::parseResponse(json, plain, synced));

    // Invalid JSON
    QByteArray badJson = "{{{{";
    assert(!LyricsProvider::parseResponse(badJson, plain, synced));
    std::cout << "  [PASS] LyricsProvider::parseResponse (empty/invalid)\n";
}

// ---------------------------------------------------------------------------
// MetadataProvider multi-result tests
// ---------------------------------------------------------------------------
static void testMetadataProviderParseAllResults() {
    QByteArray json = R"JSON({
        "recordings": [
            {
                "id": "rec-1",
                "title": "Bohemian Rhapsody",
                "score": 100,
                "artist-credit": [{"artist": {"name": "Queen"}}],
                "releases": [{"id": "rel-a", "title": "A Night at the Opera"}, {"id": "rel-b", "title": "Greatest Hits"}]
            },
            {
                "id": "rec-2",
                "title": "Bohemian Rhapsody Live",
                "score": 85,
                "artist-credit": [{"artist": {"name": "Queen"}}],
                "releases": [{"id": "rel-c", "title": "Live at Wembley"}]
            }
        ]
    })JSON";

    QList<MetadataResult> results = MetadataProvider::parseAllResults(json);
    // rec-1 has 2 releases, rec-2 has 1 → 3 results total
    assert(results.size() == 3);
    assert(results[0].title == "Bohemian Rhapsody");
    assert(results[0].artist == "Queen");
    assert(results[0].album == "A Night at the Opera");
    assert(results[0].recordingId == "rec-1");
    assert(results[0].releaseId == "rel-a");
    assert(results[0].score == 100);
    assert(results[1].title == "Bohemian Rhapsody");
    assert(results[1].album == "Greatest Hits");
    assert(results[1].recordingId == "rec-1");
    assert(results[1].releaseId == "rel-b");
    assert(results[2].title == "Bohemian Rhapsody Live");
    assert(results[2].album == "Live at Wembley");
    assert(results[2].releaseId == "rel-c");
    assert(results[2].score == 85);
    std::cout << "  [PASS] MetadataProvider::parseAllResults (multiple releases expanded)\n";
}

static void testMetadataProviderToVariantList() {
    QList<MetadataResult> results;
    MetadataResult r;
    r.title = "Test";
    r.artist = "Artist";
    r.album = "Album";
    r.recordingId = "id-1";
    r.score = 90;
    results.append(r);

    QVariantList vl = MetadataProvider::toVariantList(results);
    assert(vl.size() == 1);
    QVariantMap m = vl[0].toMap();
    assert(m["title"].toString() == "Test");
    assert(m["artist"].toString() == "Artist");
    assert(m["album"].toString() == "Album");
    assert(m["score"].toInt() == 90);
    std::cout << "  [PASS] MetadataProvider::toVariantList\n";
}

// ---------------------------------------------------------------------------
// AudioFingerprinter unit tests (parsing only – no fpcalc needed)
// ---------------------------------------------------------------------------
static void testFingerprintParseFpcalcOutput() {
    QByteArray output = "DURATION=240\nFINGERPRINT=AQADtMkUaUkSRZg\n";
    int duration = 0;
    QString fingerprint;
    assert(AudioFingerprinter::parseFpcalcOutput(output, duration, fingerprint));
    assert(duration == 240);
    assert(fingerprint == "AQADtMkUaUkSRZg");
    std::cout << "  [PASS] AudioFingerprinter::parseFpcalcOutput (valid)\n";
}

static void testFingerprintParseFpcalcOutputInvalid() {
    QByteArray output = "INVALID=DATA\n";
    int duration = 0;
    QString fingerprint;
    assert(!AudioFingerprinter::parseFpcalcOutput(output, duration, fingerprint));
    std::cout << "  [PASS] AudioFingerprinter::parseFpcalcOutput (invalid)\n";
}

static void testFingerprintParseFpcalcOutputEmpty() {
    QByteArray output = "";
    int duration = 0;
    QString fingerprint;
    assert(!AudioFingerprinter::parseFpcalcOutput(output, duration, fingerprint));
    std::cout << "  [PASS] AudioFingerprinter::parseFpcalcOutput (empty)\n";
}

// ---------------------------------------------------------------------------
// AcoustIdClient unit tests (URL building & JSON parsing – no network)
// ---------------------------------------------------------------------------
static void testAcoustIdBuildUrl() {
    QUrl url = AcoustIdClient::buildLookupUrl(240, "AQADtMkUaUkSRZg", "testkey");
    assert(url.host() == "api.acoustid.org");
    assert(url.path() == "/v2/lookup");
    QUrlQuery q(url);
    assert(q.queryItemValue("client") == "testkey");
    assert(q.queryItemValue("duration") == "240");
    assert(q.queryItemValue("fingerprint") == "AQADtMkUaUkSRZg");
    assert(q.hasQueryItem("meta"));
    std::cout << "  [PASS] AcoustIdClient::buildLookupUrl\n";
}

static void testAcoustIdParseResponse() {
    QByteArray json = R"({
        "status": "ok",
        "results": [{
            "score": 0.95,
            "recordings": [{
                "id": "mbid-123",
                "title": "Test Song",
                "artists": [{"name": "Test Artist"}],
                "releasegroups": [{"title": "Test Album"}]
            }]
        }]
    })";

    QList<AcoustIdResult> results = AcoustIdClient::parseResponse(json);
    assert(results.size() == 1);
    assert(results[0].title == "Test Song");
    assert(results[0].artist == "Test Artist");
    assert(results[0].album == "Test Album");
    assert(results[0].recordingId == "mbid-123");
    assert(results[0].score > 0.9);
    std::cout << "  [PASS] AcoustIdClient::parseResponse (valid)\n";
}

static void testAcoustIdParseResponseEmpty() {
    QByteArray json = R"({"status": "ok", "results": []})";
    QList<AcoustIdResult> results = AcoustIdClient::parseResponse(json);
    assert(results.isEmpty());

    QByteArray badJson = "not json";
    assert(AcoustIdClient::parseResponse(badJson).isEmpty());
    std::cout << "  [PASS] AcoustIdClient::parseResponse (empty/invalid)\n";
}

static void testAcoustIdParseResponseMultiple() {
    QByteArray json = R"({
        "status": "ok",
        "results": [{
            "score": 0.95,
            "recordings": [
                {
                    "id": "mbid-1",
                    "title": "Song A",
                    "artists": [{"name": "Artist A"}],
                    "releasegroups": [{"title": "Album A"}]
                },
                {
                    "id": "mbid-2",
                    "title": "Song B",
                    "artists": [{"name": "Artist B"}],
                    "releasegroups": [{"title": "Album B"}]
                }
            ]
        }]
    })";

    QList<AcoustIdResult> results = AcoustIdClient::parseResponse(json);
    assert(results.size() == 2);
    assert(results[0].title == "Song A");
    assert(results[1].title == "Song B");
    std::cout << "  [PASS] AcoustIdClient::parseResponse (multiple recordings)\n";
}

static void testAcoustIdToVariantList() {
    QList<AcoustIdResult> results;
    AcoustIdResult r;
    r.recordingId = "id-1";
    r.title = "T";
    r.artist = "A";
    r.album = "Al";
    r.score = 0.8;
    results.append(r);

    QVariantList vl = AcoustIdClient::toVariantList(results);
    assert(vl.size() == 1);
    QVariantMap m = vl[0].toMap();
    assert(m["title"].toString() == "T");
    assert(m["artist"].toString() == "A");
    assert(m["score"].toDouble() > 0.7);
    std::cout << "  [PASS] AcoustIdClient::toVariantList\n";
}

// ---------------------------------------------------------------------------
// MetadataWriter unit tests
// ---------------------------------------------------------------------------
static void testMetadataWriterIsSupported() {
#ifdef MSCPLAYER_HAS_TAGLIB
    assert(MetadataWriter::isSupported() == true);
    std::cout << "  [PASS] MetadataWriter::isSupported (TagLib available)\n";
#else
    assert(MetadataWriter::isSupported() == false);
    std::cout << "  [PASS] MetadataWriter::isSupported (TagLib not available)\n";
#endif
}

// ---------------------------------------------------------------------------
// CoverArtProvider unit tests (URL building – no network)
// ---------------------------------------------------------------------------
static void testCoverArtBuildReleaseUrl() {
    QUrl url = CoverArtProvider::buildReleaseUrl("12345678-abcd-efgh-ijkl-000000000000");
    assert(url.host() == "coverartarchive.org");
    assert(url.path() == "/release/12345678-abcd-efgh-ijkl-000000000000/front-250");
    std::cout << "  [PASS] CoverArtProvider::buildReleaseUrl\n";
}

static void testCoverArtBuildReleaseGroupUrl() {
    QUrl url = CoverArtProvider::buildReleaseGroupUrl("abcd1234-0000-0000-0000-000000000000");
    assert(url.host() == "coverartarchive.org");
    assert(url.path() == "/release-group/abcd1234-0000-0000-0000-000000000000/front-250");
    std::cout << "  [PASS] CoverArtProvider::buildReleaseGroupUrl\n";
}

// ---------------------------------------------------------------------------
// MetadataProvider releaseId extraction test
// ---------------------------------------------------------------------------
static void testMetadataProviderParseReleaseId() {
    QByteArray json = R"JSON({
        "recordings": [{
            "id": "rec-1",
            "title": "Test Song",
            "score": 95,
            "artist-credit": [{"artist": {"name": "Test Artist"}}],
            "releases": [{"id": "release-mbid-123", "title": "Test Album"}]
        }]
    })JSON";

    QList<MetadataResult> results = MetadataProvider::parseAllResults(json);
    assert(results.size() == 1);
    assert(results[0].releaseId == "release-mbid-123");
    std::cout << "  [PASS] MetadataProvider releaseId extraction\n";
}

static void testMetadataProviderVariantListIncludesReleaseId() {
    QList<MetadataResult> results;
    MetadataResult r;
    r.title = "T";
    r.artist = "A";
    r.album = "Al";
    r.recordingId = "rec-1";
    r.releaseId = "rel-1";
    r.score = 80;
    results.append(r);

    QVariantList vl = MetadataProvider::toVariantList(results);
    QVariantMap m = vl[0].toMap();
    assert(m["releaseId"].toString() == "rel-1");
    assert(m.contains("coverArtUrl"));
    assert(m["coverArtUrl"].toString().contains("rel-1"));
    assert(m["coverArtUrl"].toString().contains("coverartarchive.org/release/"));
    std::cout << "  [PASS] MetadataProvider::toVariantList includes releaseId & coverArtUrl\n";
}

static void testMetadataProviderVariantListNoCoverArtUrlWithoutRelease() {
    QList<MetadataResult> results;
    MetadataResult r;
    r.title = "T";
    r.artist = "A";
    r.recordingId = "rec-1";
    r.score = 80;
    // No releaseId
    results.append(r);

    QVariantList vl = MetadataProvider::toVariantList(results);
    QVariantMap m = vl[0].toMap();
    assert(!m.contains("coverArtUrl"));
    std::cout << "  [PASS] MetadataProvider::toVariantList omits coverArtUrl when no releaseId\n";
}

// ---------------------------------------------------------------------------
// AcoustIdClient releaseGroupId extraction test
// ---------------------------------------------------------------------------
static void testAcoustIdParseReleaseGroupId() {
    QByteArray json = R"JSON({
        "status": "ok",
        "results": [{
            "score": 0.95,
            "recordings": [{
                "id": "mbid-123",
                "title": "Test Song",
                "artists": [{"name": "Test Artist"}],
                "releasegroups": [{"id": "rg-mbid-456", "title": "Test Album"}]
            }]
        }]
    })JSON";

    QList<AcoustIdResult> results = AcoustIdClient::parseResponse(json);
    assert(results.size() == 1);
    assert(results[0].releaseGroupId == "rg-mbid-456");
    std::cout << "  [PASS] AcoustIdClient releaseGroupId extraction\n";
}

static void testAcoustIdVariantListIncludesReleaseGroupId() {
    QList<AcoustIdResult> results;
    AcoustIdResult r;
    r.recordingId = "id-1";
    r.title = "T";
    r.artist = "A";
    r.album = "Al";
    r.releaseGroupId = "rg-1";
    r.score = 0.8;
    results.append(r);

    QVariantList vl = AcoustIdClient::toVariantList(results);
    QVariantMap m = vl[0].toMap();
    assert(m["releaseGroupId"].toString() == "rg-1");
    assert(m.contains("coverArtUrl"));
    assert(m["coverArtUrl"].toString().contains("rg-1"));
    assert(m["coverArtUrl"].toString().contains("coverartarchive.org/release-group/"));
    std::cout << "  [PASS] AcoustIdClient::toVariantList includes releaseGroupId & coverArtUrl\n";
}

// ---------------------------------------------------------------------------
// Multi-release expansion tests
// ---------------------------------------------------------------------------
static void testMetadataProviderMultiReleasesCapped() {
    // Build a recording with 8 releases; only first 5 should be expanded.
    QByteArray json = R"JSON({
        "recordings": [{
            "id": "rec-1",
            "title": "Song",
            "score": 90,
            "artist-credit": [{"artist": {"name": "Artist"}}],
            "releases": [
                {"id": "r1", "title": "A1"},
                {"id": "r2", "title": "A2"},
                {"id": "r3", "title": "A3"},
                {"id": "r4", "title": "A4"},
                {"id": "r5", "title": "A5"},
                {"id": "r6", "title": "A6"},
                {"id": "r7", "title": "A7"},
                {"id": "r8", "title": "A8"}
            ]
        }]
    })JSON";

    QList<MetadataResult> results = MetadataProvider::parseAllResults(json);
    assert(results.size() == 5);
    assert(results[0].album == "A1");
    assert(results[4].album == "A5");
    std::cout << "  [PASS] MetadataProvider multi-release capped at 5\n";
}

static void testMetadataProviderNoReleases() {
    QByteArray json = R"JSON({
        "recordings": [{
            "id": "rec-1",
            "title": "Unknown Song",
            "score": 70,
            "artist-credit": [{"artist": {"name": "Artist"}}]
        }]
    })JSON";

    QList<MetadataResult> results = MetadataProvider::parseAllResults(json);
    assert(results.size() == 1);
    assert(results[0].title == "Unknown Song");
    assert(results[0].album.isEmpty());
    assert(results[0].releaseId.isEmpty());
    std::cout << "  [PASS] MetadataProvider recording with no releases\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\nRunning Playlist tests...\n";
    testPlaylistBasic();
    testPlaylistAddAndCurrent();
    testPlaylistNextPrev();
    testPlaylistRemove();
    testPlaylistClear();
    testPlaylistSetCurrentIndex();
    testPlaylistUnicodePaths();

    std::cout << "\nRunning MetadataProvider tests...\n";
    testMetadataProviderBuildUrl();
    testMetadataProviderBuildUrlWithLimit();
    testMetadataProviderParseResponse();
    testMetadataProviderParseEmpty();
    testMetadataProviderParseAllResults();
    testMetadataProviderToVariantList();

    std::cout << "\nRunning LyricsProvider tests...\n";
    testLyricsProviderBuildUrl();
    testLyricsProviderBuildUrlNoArtist();
    testLyricsProviderParseResponse();
    testLyricsProviderParseEmpty();

    std::cout << "\nRunning AudioFingerprinter tests...\n";
    testFingerprintParseFpcalcOutput();
    testFingerprintParseFpcalcOutputInvalid();
    testFingerprintParseFpcalcOutputEmpty();

    std::cout << "\nRunning AcoustIdClient tests...\n";
    testAcoustIdBuildUrl();
    testAcoustIdParseResponse();
    testAcoustIdParseResponseEmpty();
    testAcoustIdParseResponseMultiple();
    testAcoustIdToVariantList();

    std::cout << "\nRunning MetadataWriter tests...\n";
    testMetadataWriterIsSupported();

    std::cout << "\nRunning CoverArtProvider tests...\n";
    testCoverArtBuildReleaseUrl();
    testCoverArtBuildReleaseGroupUrl();

    std::cout << "\nRunning releaseId / releaseGroupId extraction tests...\n";
    testMetadataProviderParseReleaseId();
    testMetadataProviderVariantListIncludesReleaseId();
    testMetadataProviderVariantListNoCoverArtUrlWithoutRelease();
    testAcoustIdParseReleaseGroupId();
    testAcoustIdVariantListIncludesReleaseGroupId();

    std::cout << "\nRunning multi-release expansion tests...\n";
    testMetadataProviderMultiReleasesCapped();
    testMetadataProviderNoReleases();

    std::cout << "\nAll tests passed!\n";
    return 0;
}
