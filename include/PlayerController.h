// PlayerController.h
// QObject bridge between the QML front-end and the miniaudio back-end.

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QUrl>
#include "AudioPlayer.h"
#include "Playlist.h"
#include "MetadataProvider.h"
#include "LyricsProvider.h"

class PlayerController : public QObject {
    Q_OBJECT

    // Properties exposed to QML
    Q_PROPERTY(bool        isPlaying    READ isPlaying    NOTIFY playingChanged)
    Q_PROPERTY(float       volume       READ volume       WRITE setVolume   NOTIFY volumeChanged)
    Q_PROPERTY(QString     currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QStringList trackList    READ trackList    NOTIFY playlistChanged)
    Q_PROPERTY(int         trackCount   READ trackCount   NOTIFY playlistChanged)
    Q_PROPERTY(int         currentIndex READ currentIndex NOTIFY currentTrackChanged)
    Q_PROPERTY(float       position     READ position     NOTIFY positionChanged)
    Q_PROPERTY(float       duration     READ duration     NOTIFY durationChanged)

    // Online metadata properties
    Q_PROPERTY(QString trackTitle  READ trackTitle  NOTIFY metadataChanged)
    Q_PROPERTY(QString trackArtist READ trackArtist NOTIFY metadataChanged)
    Q_PROPERTY(QString trackAlbum  READ trackAlbum  NOTIFY metadataChanged)
    Q_PROPERTY(QString lyrics      READ lyrics      NOTIFY lyricsChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    bool        isPlaying()    const;
    float       volume()       const;
    void        setVolume(float v);
    QString     currentTrack() const;
    QStringList trackList()    const;
    int         trackCount()   const;
    int         currentIndex() const;
    float       position()     const;
    float       duration()     const;

    // Online metadata accessors
    QString trackTitle()  const;
    QString trackArtist() const;
    QString trackAlbum()  const;
    QString lyrics()      const;

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

signals:
    void playingChanged();
    void volumeChanged();
    void currentTrackChanged();
    void playlistChanged();
    void positionChanged();
    void durationChanged();
    void metadataChanged();
    void lyricsChanged();

private slots:
    void onMetadataReady(const QString &artist, const QString &album,
                         const QString &title);
    void onLyricsReady(const QString &plainLyrics, const QString &syncedLyrics);

private:
    void lookupTrackInfo();
    void clearMetadata();

    AudioPlayer      m_player;
    Playlist         m_playlist;
    MetadataProvider m_metadataProvider;
    LyricsProvider   m_lyricsProvider;

    QString m_trackTitle;
    QString m_trackArtist;
    QString m_trackAlbum;
    QString m_lyrics;
};

#endif // PLAYERCONTROLLER_H