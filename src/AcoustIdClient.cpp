// AcoustIdClient.cpp
// AcoustID fingerprint lookup implementation.
// API docs: https://acoustid.org/webservice

#include "AcoustIdClient.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

AcoustIdClient::AcoustIdClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_nam, &QNetworkAccessManager::finished, this,
            &AcoustIdClient::onReplyFinished);
}

void AcoustIdClient::lookup(int duration, const QString &fingerprint)
{
    if (fingerprint.isEmpty()) {
        emit lookupFailed(QStringLiteral("Empty fingerprint"));
        return;
    }

    QUrl url = buildLookupUrl(duration, fingerprint, QLatin1String(kDefaultApiKey));
    QNetworkRequest request(url);
    request.setRawHeader(
        "User-Agent", "MSCPLAYER/0.1.0 (https://github.com/MollycodeX/MSCPLAYER)");
    request.setRawHeader("Accept", "application/json");
    m_nam.get(request);
}

QUrl AcoustIdClient::buildLookupUrl(int duration, const QString &fingerprint,
                                    const QString &clientApiKey)
{
    QUrl url(QStringLiteral("https://api.acoustid.org/v2/lookup"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("client"), clientApiKey);
    query.addQueryItem(QStringLiteral("duration"), QString::number(duration));
    query.addQueryItem(QStringLiteral("fingerprint"), fingerprint);
    // Request recording metadata including releases (albums).
    query.addQueryItem(QStringLiteral("meta"),
                       QStringLiteral("recordings releasegroups"));
    url.setQuery(query);
    return url;
}

QList<AcoustIdResult> AcoustIdClient::parseResponse(const QByteArray &data)
{
    QList<AcoustIdResult> results;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return results;

    QJsonObject root = doc.object();
    if (root.value(QStringLiteral("status")).toString() != QStringLiteral("ok"))
        return results;

    QJsonArray acoustResults = root.value(QStringLiteral("results")).toArray();
    for (const QJsonValue &rv : acoustResults) {
        QJsonObject rObj = rv.toObject();
        double score = rObj.value(QStringLiteral("score")).toDouble();

        QJsonArray recordings =
            rObj.value(QStringLiteral("recordings")).toArray();
        for (const QJsonValue &recVal : recordings) {
            QJsonObject rec = recVal.toObject();

            AcoustIdResult r;
            r.score = score;
            r.recordingId = rec.value(QStringLiteral("id")).toString();
            r.title = rec.value(QStringLiteral("title")).toString();

            // Artists
            QJsonArray artists = rec.value(QStringLiteral("artists")).toArray();
            if (!artists.isEmpty()) {
                r.artist =
                    artists.first().toObject().value(QStringLiteral("name")).toString();
            }

            // Release groups → album
            QJsonArray releaseGroups =
                rec.value(QStringLiteral("releasegroups")).toArray();
            if (!releaseGroups.isEmpty()) {
                QJsonObject rgObj = releaseGroups.first().toObject();
                r.album = rgObj.value(QStringLiteral("title")).toString();
                r.releaseGroupId = rgObj.value(QStringLiteral("id")).toString();
            }

            if (!r.title.isEmpty())
                results.append(r);
        }
    }

    return results;
}

QVariantList AcoustIdClient::toVariantList(const QList<AcoustIdResult> &results)
{
    QVariantList list;
    for (const auto &r : results) {
        QVariantMap map;
        map[QStringLiteral("recordingId")] = r.recordingId;
        map[QStringLiteral("title")] = r.title;
        map[QStringLiteral("artist")] = r.artist;
        map[QStringLiteral("album")] = r.album;
        map[QStringLiteral("releaseGroupId")] = r.releaseGroupId;
        map[QStringLiteral("score")] = r.score;
        if (!r.releaseGroupId.isEmpty()) {
            map[QStringLiteral("coverArtUrl")] =
                QStringLiteral(
                    "https://coverartarchive.org/release-group/%1/front-250")
                    .arg(r.releaseGroupId);
        }
        list.append(map);
    }
    return list;
}

void AcoustIdClient::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit lookupFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QList<AcoustIdResult> list = parseResponse(data);
    if (list.isEmpty()) {
        emit lookupFailed(QStringLiteral("No recordings found via AcoustID"));
    } else {
        emit resultsReady(toVariantList(list));
    }
}
