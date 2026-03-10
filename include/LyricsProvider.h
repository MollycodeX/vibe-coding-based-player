// LyricsProvider.h
// Asynchronous lyrics lookup via the LRCLIB open-source lyrics database.
// Fetches plain-text and synced (LRC) lyrics for a given track.

#ifndef LYRICSPROVIDER_H
#define LYRICSPROVIDER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class LyricsProvider : public QObject {
    Q_OBJECT

public:
    explicit LyricsProvider(QObject *parent = nullptr);

    // Triggers an asynchronous lyrics lookup by track name and artist.
    void lookup(const QString &trackName, const QString &artistName);

    // Returns the plain-text lyrics from the most recent lookup.
    QString plainLyrics() const;
    // Returns the synced (LRC format) lyrics from the most recent lookup.
    QString syncedLyrics() const;

    // Constructs the LRCLIB search URL for a given track/artist query.
    static QUrl buildSearchUrl(const QString &trackName,
                               const QString &artistName);

    // Parses a LRCLIB JSON response and extracts lyrics.
    // Returns true on success and populates the output parameters.
    static bool parseResponse(const QByteArray &data, QString &plainLyrics,
                              QString &syncedLyrics);

signals:
    void lyricsReady(const QString &plainLyrics, const QString &syncedLyrics);
    void lookupFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    QString m_plainLyrics;
    QString m_syncedLyrics;
};

#endif // LYRICSPROVIDER_H
