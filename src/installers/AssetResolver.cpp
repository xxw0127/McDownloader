#include "installers/AssetResolver.h"

#include <QJsonValue>

QVector<AssetFile> AssetResolver::parseIndex(const QJsonObject& indexJson, const QString& versionId)
{
    Q_UNUSED(versionId)

    QVector<AssetFile> result;
    const bool virtualAsset = indexJson.value(QStringLiteral("virtual")).toBool(false);
    const bool mapToResources = indexJson.value(QStringLiteral("map_to_resources")).toBool(false);

    const QJsonObject objects = indexJson.value(QStringLiteral("objects")).toObject();
    result.reserve(objects.size());

    for (auto it = objects.begin(); it != objects.end(); ++it)
    {
        const QJsonObject object = it.value().toObject();
        const QString hash = object.value(QStringLiteral("hash")).toString();
        if (hash.size() < 2)
            continue;

        AssetFile asset;
        asset.hash = hash;
        asset.size = object.value(QStringLiteral("size")).toVariant().toLongLong();
        asset.legacyPath = it.key();
        asset.relativePath = QStringLiteral("assets/objects/%1/%2").arg(hash.left(2), hash);
        asset.virtualAsset = virtualAsset;
        asset.mapToResources = mapToResources;
        result.append(asset);
    }

    return result;
}
