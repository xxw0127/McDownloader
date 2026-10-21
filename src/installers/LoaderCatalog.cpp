#include "installers/LoaderCatalog.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>

#include <algorithm>

namespace
{
void setError(QString* error, const QString& message)
{
    if (error != nullptr)
        *error = message;
}

QUrl fabricProfileUrl(const QString& mcVersion, const QString& loaderVersion)
{
    return QUrl(QStringLiteral("https://meta.fabricmc.net/v2/versions/loader/%1/%2/profile/json")
                    .arg(mcVersion, loaderVersion));
}

QUrl quiltProfileUrl(const QString& mcVersion, const QString& loaderVersion)
{
    return QUrl(QStringLiteral("https://meta.quiltmc.org/v3/versions/loader/%1/%2/profile/json")
                    .arg(mcVersion, loaderVersion));
}
}

QList<QUrl> LoaderCatalog::indexUrls(LoaderKind kind, const QString& mcVersion)
{
    if (kind == LoaderKind::Fabric)
    {
        return {
            QUrl(QStringLiteral("https://meta.fabricmc.net/v2/versions")),
            QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/fabric-meta/v2/versions"))
        };
    }
    if (kind == LoaderKind::Quilt)
    {
        return {QUrl(QStringLiteral("https://meta.quiltmc.org/v3/versions/loader/%1").arg(mcVersion))};
    }
    if (kind == LoaderKind::Forge)
    {
        return {QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/forge/minecraft/%1").arg(mcVersion))};
    }
    return {};
}

QVector<LoaderVersion> LoaderCatalog::parseIndex(LoaderKind kind,
                                                 const QByteArray& body,
                                                 const QString& mcVersion,
                                                 QString* error)
{
    if (error != nullptr)
        error->clear();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        setError(error, QStringLiteral("加载器列表 JSON 无效：%1").arg(parseError.errorString()));
        return {};
    }

    QVector<LoaderVersion> result;

    if (kind == LoaderKind::Fabric)
    {
        if (!document.isObject())
        {
            setError(error, QStringLiteral("Fabric 列表不是 JSON 对象"));
            return {};
        }

        const QJsonObject root = document.object();
        const QJsonArray games = root.value(QStringLiteral("game")).toArray();
        if (!games.isEmpty())
        {
            bool supported = false;
            for (const QJsonValue& value : games)
            {
                if (value.toObject().value(QStringLiteral("version")).toString() == mcVersion)
                {
                    supported = true;
                    break;
                }
            }
            if (!supported)
            {
                setError(error, QStringLiteral("Fabric 不支持 Minecraft %1").arg(mcVersion));
                return {};
            }
        }

        const QJsonArray loaders = root.value(QStringLiteral("loader")).toArray();
        for (const QJsonValue& value : loaders)
        {
            const QJsonObject object = value.toObject();
            LoaderVersion version;
            version.version = object.value(QStringLiteral("version")).toString();
            version.stable = object.value(QStringLiteral("stable")).toBool() ? QStringLiteral("stable")
                                                                             : QStringLiteral("beta");
            if (!version.version.isEmpty())
                result.append(version);
        }
    }
    else if (kind == LoaderKind::Quilt)
    {
        if (!document.isArray())
        {
            setError(error, QStringLiteral("Quilt 列表不是 JSON 数组"));
            return {};
        }

        const QJsonArray loaders = document.array();
        for (const QJsonValue& value : loaders)
        {
            const QJsonObject loader = value.toObject().value(QStringLiteral("loader")).toObject();
            LoaderVersion version;
            version.version = loader.value(QStringLiteral("version")).toString();
            version.stable = loader.value(QStringLiteral("stable")).toBool() ? QStringLiteral("stable")
                                                                             : QStringLiteral("beta");
            if (!version.version.isEmpty())
                result.append(version);
        }
    }
    else if (kind == LoaderKind::Forge)
    {
        if (!document.isArray())
        {
            setError(error, QStringLiteral("Forge 列表不是 JSON 数组"));
            return {};
        }

        const QJsonArray loaders = document.array();
        for (const QJsonValue& value : loaders)
        {
            const QJsonObject object = value.toObject();
            LoaderVersion version;
            version.version = object.value(QStringLiteral("version")).toString();
            version.stable = QStringLiteral("forge");
            version.released = QDateTime::fromString(object.value(QStringLiteral("modified")).toString(), Qt::ISODate);
            if (!version.version.isEmpty())
                result.append(version);
        }

        std::sort(result.begin(), result.end(), [](const LoaderVersion& left, const LoaderVersion& right) {
            return QVersionNumber::compare(QVersionNumber::fromString(left.version),
                                           QVersionNumber::fromString(right.version)) > 0;
        });
    }
    else
    {
        setError(error, QStringLiteral("该加载器不支持版本列表"));
        return {};
    }

    if (result.isEmpty())
        setError(error, QStringLiteral("没有可用的加载器版本"));

    return result;
}

QList<QUrl> LoaderCatalog::profileUrls(LoaderKind kind,
                                       const QString& mcVersion,
                                       const QString& loaderVersion)
{
    if (kind == LoaderKind::Fabric)
    {
        return {
            fabricProfileUrl(mcVersion, loaderVersion),
            QUrl(QStringLiteral("https://bmclapi2.bangbang93.com/fabric-meta/v2/versions/loader/%1/%2/profile/json")
                     .arg(mcVersion, loaderVersion))
        };
    }
    if (kind == LoaderKind::Quilt)
        return {quiltProfileUrl(mcVersion, loaderVersion)};
    return {};
}
