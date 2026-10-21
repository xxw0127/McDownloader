#include "core/VersionManifest.h"

#include <QDateTime>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUrl>

#include <algorithm>

namespace
{
void setError(QString* error, const QString& message)
{
    if (error != nullptr)
        *error = message;
}

QString displayIdFor(const QString& id)
{
    if (id == QLatin1String("20w14infinite"))
        return QStringLiteral("20w14∞");
    return id;
}

QDateTime parseTime(const QString& text)
{
    QDateTime time = QDateTime::fromString(text, Qt::ISODate);
    if (!time.isValid())
        time = QDateTime::fromString(text, Qt::ISODateWithMs);
    return time;
}
}

QVector<VersionInfo> VersionManifest::parse(const QByteArray& json, QString* error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
    {
        setError(error, QStringLiteral("版本清单不是有效 JSON：%1").arg(parseError.errorString()));
        return {};
    }

    const QJsonValue versionsValue = document.object().value(QStringLiteral("versions"));
    if (!versionsValue.isArray())
    {
        setError(error, QStringLiteral("版本清单缺少 versions 数组"));
        return {};
    }

    QVector<VersionInfo> result;
    const QJsonArray versions = versionsValue.toArray();
    result.reserve(versions.size());

    for (const QJsonValue& item : versions)
    {
        if (!item.isObject())
            continue;

        const QJsonObject object = item.toObject();
        VersionInfo version;
        version.id = object.value(QStringLiteral("id")).toString();
        version.type = object.value(QStringLiteral("type")).toString();
        version.releaseTime = parseTime(object.value(QStringLiteral("releaseTime")).toString());
        version.url = QUrl(object.value(QStringLiteral("url")).toString());
        version.lore = object.value(QStringLiteral("lore")).toString();

        if (version.id.isEmpty())
            continue;

        version.displayId = displayIdFor(version.id);
        result.append(version);
    }

    std::sort(result.begin(), result.end(), [](const VersionInfo& left, const VersionInfo& right) {
        if (left.releaseTime == right.releaseTime)
            return left.id < right.id;
        return left.releaseTime > right.releaseTime;
    });

    if (error != nullptr)
        error->clear();
    return result;
}

QVector<VersionInfo> VersionManifest::merge(const QVector<VersionInfo>& primary,
                                            const QVector<VersionInfo>& secondary)
{
    QHash<QString, VersionInfo> byId;

    for (const VersionInfo& version : primary)
        byId.insert(version.id, version);

    for (const VersionInfo& version : secondary)
    {
        auto it = byId.find(version.id);
        if (it == byId.end())
        {
            byId.insert(version.id, version);
            continue;
        }

        // 时间较新的胜出；时间相同时优先 secondary（调用方约定为官方源）。
        if (version.releaseTime >= it.value().releaseTime)
            it.value() = version;
    }

    QVector<VersionInfo> result;
    result.reserve(byId.size());
    for (const VersionInfo& version : byId)
        result.append(version);

    std::sort(result.begin(), result.end(), [](const VersionInfo& left, const VersionInfo& right) {
        if (left.releaseTime == right.releaseTime)
            return left.id < right.id;
        return left.releaseTime > right.releaseTime;
    });

    return result;
}
