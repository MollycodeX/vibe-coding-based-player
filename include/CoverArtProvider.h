// CoverArtProvider.h
// Asynchronous album cover art fetcher using the MusicBrainz Cover Art Archive.
// https://musicbrainz.org/doc/Cover_Art_Archive/API

#ifndef COVERARTPROVIDER_H
#define COVERARTPROVIDER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class CoverArtProvider : public QObject {
    Q_OBJECT

public:
    explicit CoverArtProvider(QObject *parent = nullptr);

    /// Fetch cover art for a MusicBrainz release MBID.
    void fetchByReleaseId(const QString &releaseId);

    /// Fetch cover art for a MusicBrainz release-group MBID.
    void fetchByReleaseGroupId(const QString &releaseGroupId);

    /// Build the Cover Art Archive URL for a release.
    static QUrl buildReleaseUrl(const QString &releaseId);

    /// Build the Cover Art Archive URL for a release group.
    static QUrl buildReleaseGroupUrl(const QString &releaseGroupId);

signals:
    void coverArtReady(const QByteArray &imageData);
    void coverArtFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    void fetchUrl(const QUrl &url);
    QNetworkAccessManager m_nam;
};

#endif // COVERARTPROVIDER_H
