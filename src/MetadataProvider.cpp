// MetadataProvider.cpp
// MusicBrainz-based metadata lookup implementation.
// API docs: https://musicbrainz.org/doc/MusicBrainz_API

#include "MetadataProvider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

MetadataProvider::MetadataProvider(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this,
            &MetadataProvider::onReplyFinished);
}

QUrl MetadataProvider::buildSearchUrl(const QString &trackName)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/recording"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("query"), trackName);
    query.addQueryItem(QStringLiteral("fmt"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("limit"), QStringLiteral("1"));
    url.setQuery(query);
    return url;
}

bool MetadataProvider::parseResponse(const QByteArray &data, QString &artist,
                                     QString &album, QString &title)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    QJsonArray recordings = doc.object().value(QStringLiteral("recordings")).toArray();
    if (recordings.isEmpty())
        return false;

    QJsonObject rec = recordings.first().toObject();

    // Title
    title = rec.value(QStringLiteral("title")).toString();

    // Artist (first artist-credit entry)
    QJsonArray credits = rec.value(QStringLiteral("artist-credit")).toArray();
    if (!credits.isEmpty()) {
        QJsonObject artistObj =
            credits.first().toObject().value(QStringLiteral("artist")).toObject();
        artist = artistObj.value(QStringLiteral("name")).toString();
    }

    // Album (first release entry)
    QJsonArray releases = rec.value(QStringLiteral("releases")).toArray();
    if (!releases.isEmpty()) {
        album = releases.first().toObject().value(QStringLiteral("title")).toString();
    }

    return !title.isEmpty();
}

void MetadataProvider::lookup(const QString &trackName)
{
    if (trackName.isEmpty())
        return;

    QUrl url = buildSearchUrl(trackName);
    QNetworkRequest request(url);
    // MusicBrainz requires a descriptive User-Agent header.
    request.setRawHeader(
        "User-Agent", "MSCPLAYER/0.1.0 (https://github.com/MollycodeX/MSCPLAYER)");
    request.setRawHeader("Accept", "application/json");
    m_nam.get(request);
}

void MetadataProvider::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit lookupFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QString artist, album, title;
    if (parseResponse(data, artist, album, title)) {
        m_artist = artist;
        m_album = album;
        m_title = title;
        emit metadataReady(artist, album, title);
    } else {
        emit lookupFailed(QStringLiteral("No metadata found"));
    }
}

QString MetadataProvider::artist() const { return m_artist; }
QString MetadataProvider::album() const { return m_album; }
QString MetadataProvider::title() const { return m_title; }
