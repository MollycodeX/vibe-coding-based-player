// MetadataProvider.h
// Asynchronous metadata lookup via the MusicBrainz open-source database.
// Fetches artist and album information for a given track name.

#ifndef METADATAPROVIDER_H
#define METADATAPROVIDER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantList>

/// A single metadata result from a MusicBrainz recording search.
struct MetadataResult {
    QString title;
    QString artist;
    QString album;
    QString recordingId;
    QString releaseId; // MusicBrainz release MBID (used for cover art)
    int score{0};      // MusicBrainz relevance score (0-100)
};

class MetadataProvider : public QObject {
    Q_OBJECT

public:
    explicit MetadataProvider(QObject *parent = nullptr);

    // Triggers an asynchronous metadata lookup for the given track name.
    // Results are delivered via the metadataReady signal (first match)
    // and multipleResultsReady signal (all matches).
    void lookup(const QString &trackName);

    // Returns the artist from the most recent successful lookup.
    QString artist() const;
    // Returns the album from the most recent successful lookup.
    QString album() const;
    // Returns the title from the most recent successful lookup.
    QString title() const;

    // Constructs the MusicBrainz search URL for a given query.
    // The overload with a limit parameter controls how many results are fetched.
    static QUrl buildSearchUrl(const QString &trackName);
    static QUrl buildSearchUrl(const QString &trackName, int limit);

    // Parses a MusicBrainz JSON response and extracts artist, album, title
    // from the first recording. Kept for backward compatibility.
    static bool parseResponse(const QByteArray &data, QString &artist,
                              QString &album, QString &title);

    // Parses a MusicBrainz JSON response into a list of all recording results.
    static QList<MetadataResult> parseAllResults(const QByteArray &data);

    // Converts a QList<MetadataResult> to a QVariantList for QML consumption.
    static QVariantList toVariantList(const QList<MetadataResult> &results);

signals:
    void metadataReady(const QString &artist, const QString &album,
                       const QString &title);
    void multipleResultsReady(const QVariantList &results);
    void lookupFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    QString m_artist;
    QString m_album;
    QString m_title;

    static constexpr int kDefaultLimit = 10;
};

#endif // METADATAPROVIDER_H
