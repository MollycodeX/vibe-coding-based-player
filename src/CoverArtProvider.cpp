// CoverArtProvider.cpp
// Fetches album cover art from the MusicBrainz Cover Art Archive.
// API docs: https://musicbrainz.org/doc/Cover_Art_Archive/API

#include "CoverArtProvider.h"
#include <QNetworkRequest>

CoverArtProvider::CoverArtProvider(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this,
            &CoverArtProvider::onReplyFinished);
}

QUrl CoverArtProvider::buildReleaseUrl(const QString &releaseId)
{
    return QUrl(QStringLiteral("https://coverartarchive.org/release/%1/front-250")
                    .arg(releaseId));
}

QUrl CoverArtProvider::buildReleaseGroupUrl(const QString &releaseGroupId)
{
    return QUrl(
        QStringLiteral("https://coverartarchive.org/release-group/%1/front-250")
            .arg(releaseGroupId));
}

void CoverArtProvider::fetchByReleaseId(const QString &releaseId)
{
    if (releaseId.isEmpty()) {
        emit coverArtFailed(QStringLiteral("Empty release ID"));
        return;
    }
    fetchUrl(buildReleaseUrl(releaseId));
}

void CoverArtProvider::fetchByReleaseGroupId(const QString &releaseGroupId)
{
    if (releaseGroupId.isEmpty()) {
        emit coverArtFailed(QStringLiteral("Empty release-group ID"));
        return;
    }
    fetchUrl(buildReleaseGroupUrl(releaseGroupId));
}

void CoverArtProvider::fetchUrl(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setRawHeader(
        "User-Agent", "MSCPLAYER/0.1.0 (https://github.com/MollycodeX/MSCPLAYER)");
    // The Cover Art Archive returns a 307 redirect to the actual image file.
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
    m_nam.get(request);
}

void CoverArtProvider::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit coverArtFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        emit coverArtFailed(QStringLiteral("Empty cover art response"));
        return;
    }

    emit coverArtReady(data);
}
