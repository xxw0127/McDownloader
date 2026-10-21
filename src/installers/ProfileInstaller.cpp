#include "installers/ProfileInstaller.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

#include "download/MirrorResolver.h"

namespace
{
QString expandPlaceholders(QString value)
{
    value.replace(QStringLiteral("${arch}"), QStringLiteral("64"));
    return value;
}

QString mavenPathFor(const QString& rawName)
{
    const QString name = expandPlaceholders(rawName);
    const QStringList parts = name.split(QLatin1Char(':'));
    if (parts.size() < 3)
        return QString();

    QString groupPath = parts.at(0);
    groupPath.replace(QLatin1Char('.'), QLatin1Char('/'));
    QString fileName = parts.at(1) + QLatin1Char('-') + parts.at(2);
    if (parts.size() >= 4)
        fileName += QLatin1Char('-') + parts.at(3);
    fileName += QStringLiteral(".jar");

    return groupPath + QLatin1Char('/') + parts.at(1) + QLatin1Char('/') + parts.at(2) +
           QLatin1Char('/') + fileName;
}

qint64 jsonSize(const QJsonObject& object)
{
    if (!object.contains(QStringLiteral("size")))
        return -1;
    return object.value(QStringLiteral("size")).toVariant().toLongLong();
}
}

QString ProfileInstaller::instanceId(LoaderKind kind,
                                     const QString& mcVersion,
                                     const QString& loaderVersion)
{
    if (kind == LoaderKind::Fabric)
        return QStringLiteral("fabric-loader-%1-%2").arg(loaderVersion, mcVersion);
    if (kind == LoaderKind::Quilt)
        return QStringLiteral("quilt-loader-%1-%2").arg(loaderVersion, mcVersion);
    return QString();
}

QJsonObject ProfileInstaller::prepareProfile(const QByteArray& raw, const QString& id, QString* error)
{
    const QJsonDocument document = QJsonDocument::fromJson(raw);
    if (!document.isObject())
    {
        if (error != nullptr)
            *error = QStringLiteral("加载器 profile 不是 JSON 对象");
        return {};
    }

    QJsonObject profile = document.object();
    if (profile.value(QStringLiteral("inheritsFrom")).toString().isEmpty())
    {
        if (error != nullptr)
            *error = QStringLiteral("加载器 profile 缺少 inheritsFrom");
        return {};
    }

    profile.insert(QStringLiteral("id"), id);
    if (error != nullptr)
        error->clear();
    return profile;
}

InstallPlan ProfileInstaller::buildPlan(const QJsonObject& profile, const QString& mcDir, const QString& id)
{
    Q_UNUSED(mcDir)

    InstallPlan plan;
    plan.versionId = id;
    plan.versionJsonPath = QStringLiteral("versions/%1/%1.json").arg(id);

    const QJsonArray libraries = profile.value(QStringLiteral("libraries")).toArray();
    for (const QJsonValue& value : libraries)
    {
        const QJsonObject library = value.toObject();
        const QString name = library.value(QStringLiteral("name")).toString();
        const QString path = mavenPathFor(name);
        if (path.isEmpty())
            continue;

        FileSpec file;
        file.targetRelativePath = QStringLiteral("libraries/") + path;
        file.required = true;

        const QJsonObject artifact = library.value(QStringLiteral("downloads")).toObject()
                                         .value(QStringLiteral("artifact")).toObject();
        if (!artifact.isEmpty())
        {
            QString artifactPath = expandPlaceholders(artifact.value(QStringLiteral("path")).toString());
            if (artifactPath.isEmpty())
                artifactPath = path;
            file.targetRelativePath = QStringLiteral("libraries/") + artifactPath;
            file.size = jsonSize(artifact);
            file.sha1 = artifact.value(QStringLiteral("sha1")).toString().toLatin1();
            const QUrl official(expandPlaceholders(artifact.value(QStringLiteral("url")).toString()));
            file.urls = MirrorResolver::libraryUrls(artifactPath, official);
        }
        else
        {
            QString base = expandPlaceholders(library.value(QStringLiteral("url")).toString());
            if (base.isEmpty())
                base = QStringLiteral("https://libraries.minecraft.net/");
            if (!base.endsWith(QLatin1Char('/')))
                base += QLatin1Char('/');
            const QUrl official(base + path);
            file.urls = MirrorResolver::libraryUrls(path, official);
        }

        plan.files.append(file);
    }

    return plan;
}
