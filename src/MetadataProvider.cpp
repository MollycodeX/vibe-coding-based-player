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
    return buildSearchUrl(trackName, kDefaultLimit);
}

QUrl MetadataProvider::buildSearchUrl(const QString &trackName, int limit)
{
    QUrl url(QStringLiteral("https://musicbrainz.org/ws/2/recording"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("query"), trackName);
    query.addQueryItem(QStringLiteral("fmt"), QStringLiteral("json"));
    query.addQueryItem(QStringLiteral("limit"), QString::number(limit));
    url.setQuery(query);
    return url;
}

bool MetadataProvider::parseResponse(const QByteArray &data, QString &artist,
                                     QString &album, QString &title)
{
    QList<MetadataResult> results = parseAllResults(data);
    if (results.isEmpty())
        return false;

    const MetadataResult &first = results.first();
    title = first.title;
    artist = first.artist;
    album = first.album;
    return !title.isEmpty();
}

QList<MetadataResult> MetadataProvider::parseAllResults(const QByteArray &data)
{
    QList<MetadataResult> results;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return results;

    QJsonArray recordings =
        doc.object().value(QStringLiteral("recordings")).toArray();

    for (const QJsonValue &val : recordings) {
        QJsonObject rec = val.toObject();

        QString title = rec.value(QStringLiteral("title")).toString();
        if (title.isEmpty())
            continue;

        int score = rec.value(QStringLiteral("score")).toInt();
        QString recordingId = rec.value(QStringLiteral("id")).toString();

        // Artist (first artist-credit entry)
        QString artist;
        QJsonArray credits =
            rec.value(QStringLiteral("artist-credit")).toArray();
        if (!credits.isEmpty()) {
            QJsonObject artistObj = credits.first()
                                       .toObject()
                                       .value(QStringLiteral("artist"))
                                       .toObject();
            artist = artistObj.value(QStringLiteral("name")).toString();
        }

        // Expand each release into a separate result so users can pick
        // the correct album.  Cap per-recording releases to avoid explosion.
        QJsonArray releases = rec.value(QStringLiteral("releases")).toArray();
        if (releases.isEmpty()) {
            // No release info – still include the recording.
            MetadataResult r;
            r.title = title;
            r.score = score;
            r.recordingId = recordingId;
            r.artist = artist;
            results.append(r);
        } else {
            static constexpr int kMaxReleasesPerRecording = 5;
            int count = std::min(static_cast<int>(releases.size()),
                                 kMaxReleasesPerRecording);
            for (int i = 0; i < count; ++i) {
                QJsonObject releaseObj = releases[i].toObject();
                MetadataResult r;
                r.title = title;
                r.score = score;
                r.recordingId = recordingId;
                r.artist = artist;
                r.album = releaseObj.value(QStringLiteral("title")).toString();
                r.releaseId =
                    releaseObj.value(QStringLiteral("id")).toString();
                results.append(r);
            }
        }
    }

    return results;
}

QVariantList MetadataProvider::toVariantList(const QList<MetadataResult> &results)
{
    QVariantList list;
    for (const auto &r : results) {
        QVariantMap map;
        map[QStringLiteral("title")] = r.title;
        map[QStringLiteral("artist")] = r.artist;
        map[QStringLiteral("album")] = r.album;
        map[QStringLiteral("recordingId")] = r.recordingId;
        map[QStringLiteral("releaseId")] = r.releaseId;
        map[QStringLiteral("score")] = r.score;
        if (!r.releaseId.isEmpty()) {
            map[QStringLiteral("coverArtUrl")] =
                QStringLiteral("https://coverartarchive.org/release/%1/front-250")
                    .arg(r.releaseId);
        }
        list.append(map);
    }
    return list;
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
    QList<MetadataResult> allResults = parseAllResults(data);

    if (allResults.isEmpty()) {
        emit lookupFailed(QStringLiteral("No metadata found"));
        return;
    }

    // Always emit the multi-result signal so the UI can show a selection list.
    emit multipleResultsReady(toVariantList(allResults));

    // Also emit the legacy single-result signal with the best match.
    const MetadataResult &best = allResults.first();
    m_artist = best.artist;
    m_album = best.album;
    m_title = best.title;
    emit metadataReady(best.artist, best.album, best.title);
}

QString MetadataProvider::artist() const { return m_artist; }
QString MetadataProvider::album() const { return m_album; }
QString MetadataProvider::title() const { return m_title; }
