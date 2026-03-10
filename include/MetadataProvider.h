// MetadataProvider.h
// Asynchronous metadata lookup via the MusicBrainz open-source database.
// Fetches artist and album information for a given track name.

#ifndef METADATAPROVIDER_H
#define METADATAPROVIDER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MetadataProvider : public QObject {
    Q_OBJECT

public:
    explicit MetadataProvider(QObject *parent = nullptr);

    // Triggers an asynchronous metadata lookup for the given track name.
    // Results are delivered via the metadataReady signal.
    void lookup(const QString &trackName);

    // Returns the artist from the most recent successful lookup.
    QString artist() const;
    // Returns the album from the most recent successful lookup.
    QString album() const;
    // Returns the title from the most recent successful lookup.
    QString title() const;

    // Constructs the MusicBrainz search URL for a given query.
    static QUrl buildSearchUrl(const QString &trackName);

    // Parses a MusicBrainz JSON response and extracts artist, album, title.
    // Returns true on success and populates the output parameters.
    static bool parseResponse(const QByteArray &data, QString &artist,
                              QString &album, QString &title);

signals:
    void metadataReady(const QString &artist, const QString &album,
                       const QString &title);
    void lookupFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    QString m_artist;
    QString m_album;
    QString m_title;
};

#endif // METADATAPROVIDER_H
