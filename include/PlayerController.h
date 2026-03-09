// PlayerController.h
// QObject bridge between the QML front-end and the miniaudio back-end.

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "AudioPlayer.h"
#include "Playlist.h"

class PlayerController : public QObject {
    Q_OBJECT

    // Properties exposed to QML
    Q_PROPERTY(bool        isPlaying    READ isPlaying    NOTIFY playingChanged)
    Q_PROPERTY(float       volume       READ volume       WRITE setVolume   NOTIFY volumeChanged)
    Q_PROPERTY(QString     currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(QStringList trackList    READ trackList    NOTIFY playlistChanged)
    Q_PROPERTY(int         trackCount   READ trackCount   NOTIFY playlistChanged)
    Q_PROPERTY(int         currentIndex READ currentIndex NOTIFY currentTrackChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    bool        isPlaying()    const;
    float       volume()       const;
    void        setVolume(float v);
    QString     currentTrack() const;
    QStringList trackList()    const;
    int         trackCount()   const;
    int         currentIndex() const;

    // Invokable from QML
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void addTrack(const QString &filePath);
    Q_INVOKABLE void removeTrack(int index);
    Q_INVOKABLE void selectTrack(int index);

signals:
    void playingChanged();
    void volumeChanged();
    void currentTrackChanged();
    void playlistChanged();

private:
    AudioPlayer m_player;
    Playlist    m_playlist;
};

#endif // PLAYERCONTROLLER_H