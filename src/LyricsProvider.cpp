// LyricsProvider.cpp
// LRCLIB-based lyrics lookup implementation.
// API docs: https://lrclib.net/docs

#include "LyricsProvider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

LyricsProvider::LyricsProvider(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this,
            &LyricsProvider::onReplyFinished);
}

QUrl LyricsProvider::buildSearchUrl(const QString &trackName,
                                    const QString &artistName)
{
    QUrl url(QStringLiteral("https://lrclib.net/api/search"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("track_name"), trackName);
    if (!artistName.isEmpty())
        query.addQueryItem(QStringLiteral("artist_name"), artistName);
    url.setQuery(query);
    return url;
}

bool LyricsProvider::parseResponse(const QByteArray &data, QString &plainLyrics,
                                   QString &syncedLyrics)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
        return false;

    // LRCLIB /api/search returns a JSON array of matches.
    QJsonArray results;
    if (doc.isArray()) {
        results = doc.array();
    } else if (doc.isObject()) {
        // Single-object response (e.g. /api/get)
        results.append(doc.object());
    } else {
        return false;
    }

    if (results.isEmpty())
        return false;

    QJsonObject best = results.first().toObject();
    plainLyrics = best.value(QStringLiteral("plainLyrics")).toString();
    syncedLyrics = best.value(QStringLiteral("syncedLyrics")).toString();

    return !plainLyrics.isEmpty() || !syncedLyrics.isEmpty();
}

void LyricsProvider::lookup(const QString &trackName, const QString &artistName)
{
    if (trackName.isEmpty())
        return;

    QUrl url = buildSearchUrl(trackName, artistName);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent",
                         "MSCPLAYER/0.1.0 (https://github.com/MollycodeX/MSCPLAYER)");
    request.setRawHeader("Accept", "application/json");
    m_nam.get(request);
}

void LyricsProvider::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit lookupFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QString plain, synced;
    if (parseResponse(data, plain, synced)) {
        m_plainLyrics = plain;
        m_syncedLyrics = synced;
        emit lyricsReady(plain, synced);
    } else {
        emit lookupFailed(QStringLiteral("No lyrics found"));
    }
}

QString LyricsProvider::plainLyrics() const { return m_plainLyrics; }
QString LyricsProvider::syncedLyrics() const { return m_syncedLyrics; }
