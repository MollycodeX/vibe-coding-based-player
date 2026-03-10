// AcoustIdClient.h
// Asynchronous audio identification via the AcoustID open-source service.
// Takes an audio fingerprint + duration and returns matching recordings.

#ifndef ACOUSTIDCLIENT_H
#define ACOUSTIDCLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantList>

struct AcoustIdResult {
    QString recordingId;    // MusicBrainz recording MBID
    QString title;
    QString artist;
    QString album;
    QString releaseGroupId; // MusicBrainz release-group MBID (used for cover art)
    double score{0.0};      // AcoustID match confidence (0.0 – 1.0)
};

class AcoustIdClient : public QObject {
    Q_OBJECT

public:
    explicit AcoustIdClient(QObject *parent = nullptr);

    /// Perform an AcoustID lookup with the given duration (seconds) and
    /// Chromaprint fingerprint string.
    void lookup(int duration, const QString &fingerprint);

    /// Builds the AcoustID lookup URL.
    static QUrl buildLookupUrl(int duration, const QString &fingerprint,
                               const QString &clientApiKey);

    /// Parses the AcoustID JSON response into a list of results.
    static QList<AcoustIdResult> parseResponse(const QByteArray &data);

    /// Converts a QList<AcoustIdResult> to a QVariantList for QML consumption.
    static QVariantList toVariantList(const QList<AcoustIdResult> &results);

signals:
    void resultsReady(const QVariantList &results);
    void lookupFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_nam;
    // AcoustID client API key – register at https://acoustid.org/new-application
    static constexpr const char *kDefaultApiKey = "bBYMkUTf7A";
};

#endif // ACOUSTIDCLIENT_H
