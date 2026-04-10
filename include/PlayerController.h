// PlayerController.h
// QObject bridge between the QML front-end and the miniaudio back-end.

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include "AudioPlayer.h"
#include "Playlist.h"
#include "AudioFingerprinter.h"
#include "AcoustIdClient.h"
#include "MetadataProvider.h"
#include "LyricsProvider.h"
#include "MetadataWriter.h"
#include "CoverArtProvider.h"

class PlayerController : public QObject {
    Q_OBJECT

public:
    enum PlaybackMode {
        Sequential = 0,
        LoopAll,
        LoopOne
    };
    Q_ENUM(PlaybackMode)

    // Properties exposed to QML
    Q_PROPERTY(bool        isPlaying    READ isPlaying    NOTIFY playingChanged)
    Q_PROPERTY(float       volume       READ volume       WRITE setVolume   NOTIFY volumeChanged)
    Q_PROPERTY(QString     currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QStringList trackList    READ trackList    NOTIFY playlistChanged)
    Q_PROPERTY(int         trackCount   READ trackCount   NOTIFY playlistChanged)
    Q_PROPERTY(int         currentIndex READ currentIndex NOTIFY currentTrackChanged)
    Q_PROPERTY(float       position     READ position     NOTIFY positionChanged)
    Q_PROPERTY(float       duration     READ duration     NOTIFY durationChanged)
    Q_PROPERTY(PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(bool shuffleEnabled READ shuffleEnabled WRITE setShuffleEnabled NOTIFY shuffleEnabledChanged)

    // Online metadata properties
    Q_PROPERTY(QString trackTitle  READ trackTitle  NOTIFY metadataChanged)
    Q_PROPERTY(QString trackArtist READ trackArtist NOTIFY metadataChanged)
    Q_PROPERTY(QString trackAlbum  READ trackAlbum  NOTIFY metadataChanged)
    Q_PROPERTY(QString lyrics      READ lyrics      NOTIFY lyricsChanged)

    // Multiple-result metadata selection
    Q_PROPERTY(QVariantList metadataResults READ metadataResults NOTIFY metadataResultsChanged)
    Q_PROPERTY(bool fingerprintAvailable READ fingerprintAvailable CONSTANT)
    Q_PROPERTY(bool metadataWriteSupported READ metadataWriteSupported CONSTANT)

    // Album cover art
    Q_PROPERTY(QString albumArtUrl READ albumArtUrl NOTIFY albumArtChanged)
    Q_PROPERTY(bool embedAlbumArt READ embedAlbumArt WRITE setEmbedAlbumArt
                   NOTIFY embedAlbumArtChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);
    ~PlayerController() override;

    bool        isPlaying()    const;
    float       volume()       const;
    void        setVolume(float v);
    QString     currentTrack() const;
    QStringList trackList()    const;
    int         trackCount()   const;
    int         currentIndex() const;
    float       position()     const;
    float       duration()     const;
    PlaybackMode playbackMode() const;
    void        setPlaybackMode(PlaybackMode mode);

    bool        shuffleEnabled() const;
    void        setShuffleEnabled(bool enabled);

    // Online metadata accessors
    QString trackTitle()  const;
    QString trackArtist() const;
    QString trackAlbum()  const;
    QString lyrics()      const;

    // Multi-result support
    QVariantList metadataResults() const;
    bool fingerprintAvailable() const;
    bool metadataWriteSupported() const;

    // Album cover art
    QString albumArtUrl() const;
    bool embedAlbumArt() const;
    void setEmbedAlbumArt(bool embed);

    // Invokable from QML
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void addTrack(const QString &filePath);
    Q_INVOKABLE void addTrackUrl(const QUrl &fileUrl);
    Q_INVOKABLE void addFolder(const QString &folderPath);
    Q_INVOKABLE void addFolderUrl(const QUrl &folderUrl);
    Q_INVOKABLE void removeTrack(int index);
    Q_INVOKABLE void selectTrack(int index);
    Q_INVOKABLE void seek(float seconds);
    Q_INVOKABLE void updatePosition();

    /// Select a metadata result from the multi-result list by index.
    Q_INVOKABLE void selectMetadataResult(int index);
    /// Write the currently displayed metadata into the audio file.
    Q_INVOKABLE void writeMetadataToFile();

signals:
    void playingChanged();
    void volumeChanged();
    void currentTrackChanged();
    void playlistChanged();
    void positionChanged();
    void durationChanged();
    void playbackModeChanged();
    void shuffleEnabledChanged();
    void metadataChanged();
    void lyricsChanged();
    void metadataResultsChanged();
    void metadataWritten(bool success);
    void albumArtChanged();
    void embedAlbumArtChanged();

private slots:
    void onMetadataReady(const QString &artist, const QString &album,
                         const QString &title);
    void onMultipleResultsReady(const QVariantList &results);
    void onLyricsReady(const QString &plainLyrics, const QString &syncedLyrics);
    void onFingerprintReady(int duration, const QString &fingerprint);
    void onFingerprintFailed(const QString &errorMessage);
    void onAcoustIdResultsReady(const QVariantList &results);
    void onAcoustIdFailed(const QString &errorMessage);
    void onCoverArtReady(const QByteArray &imageData);
    void onCoverArtFailed(const QString &errorMessage);

private:
    void lookupTrackInfo();
    void clearMetadata();
    void fetchCoverArt(const QVariantMap &selected);

    AudioPlayer        m_player;
    Playlist           m_playlist;
    AudioFingerprinter m_fingerprinter;
    AcoustIdClient     m_acoustIdClient;
    MetadataProvider   m_metadataProvider;
    LyricsProvider     m_lyricsProvider;
    MetadataWriter     m_metadataWriter;
    CoverArtProvider   m_coverArtProvider;

    QString m_trackTitle;
    QString m_trackArtist;
    QString m_trackAlbum;
    QString m_lyrics;

    QVariantList m_metadataResults;
    QString m_albumArtUrl;
    QByteArray m_albumArtData;
    bool m_embedAlbumArt{false};
};

#endif // PLAYERCONTROLLER_H