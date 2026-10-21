#include "installers/VanillaPlanBuilder.h"

#include <QJsonValue>

#include "download/MirrorResolver.h"
#include "installers/AssetResolver.h"
#include "installers/LibraryResolver.h"

namespace
{
qint64 jsonSize(const QJsonObject& object)
{
    if (!object.contains(QStringLiteral("size")))
        return -1;
    return object.value(QStringLiteral("size")).toVariant().toLongLong();
}

FileSpec makeDownload(const QString& target, const QJsonObject& object, bool metaUrl)
{
    FileSpec file;
    file.targetRelativePath = target;
    file.size = jsonSize(object);
    file.sha1 = object.value(QStringLiteral("sha1")).toString().toLatin1();

    const QUrl official(object.value(QStringLiteral("url")).toString());
    if (metaUrl)
        file.urls = MirrorResolver::metaUrls(official);
    else
        file.urls = {official};
    return file;
}
}

InstallPlan VanillaPlanBuilder::build(const QJsonObject& versionJson,
                                      const QString& mcDir,
                                      const QString& versionId,
                                      const QJsonObject& assetIndex)
{
    Q_UNUSED(mcDir)

    InstallPlan plan;
    plan.versionId = versionId;
    plan.versionJsonPath = QStringLiteral("versions/%1/%1.json").arg(versionId);

    const QJsonObject downloads = versionJson.value(QStringLiteral("downloads")).toObject();
    const QJsonObject client = downloads.value(QStringLiteral("client")).toObject();
    if (!client.isEmpty() && !client.value(QStringLiteral("url")).toString().isEmpty())
    {
        plan.jarPath = QStringLiteral("versions/%1/%1.jar").arg(versionId);
        plan.files.append(makeDownload(plan.jarPath, client, true));
    }

    const QVector<LibraryFile> libraries = LibraryResolver::resolve(versionJson,
                                                                   QStringLiteral("windows"),
                                                                   QStringLiteral("x64"));
    for (const LibraryFile& library : libraries)
    {
        FileSpec file;
        file.targetRelativePath = library.relativePath;
        file.urls = library.urls;
        file.size = library.size;
        file.sha1 = library.sha1;
        plan.files.append(file);
    }

    const QJsonObject assetIndexObject = versionJson.value(QStringLiteral("assetIndex")).toObject();
    if (!assetIndexObject.isEmpty())
    {
        plan.assetIndexId = assetIndexObject.value(QStringLiteral("id")).toString();
        if (plan.assetIndexId.isEmpty())
            plan.assetIndexId = QStringLiteral("legacy");

        if (!assetIndexObject.value(QStringLiteral("url")).toString().isEmpty())
        {
            const QString target = QStringLiteral("assets/indexes/%1.json").arg(plan.assetIndexId);
            plan.files.append(makeDownload(target, assetIndexObject, true));
        }
    }
    else if (versionJson.value(QStringLiteral("assets")).isString())
    {
        plan.assetIndexId = versionJson.value(QStringLiteral("assets")).toString();
    }
    else
    {
        plan.assetIndexId = QStringLiteral("legacy");
    }

    if (!assetIndex.isEmpty())
    {
        const QVector<AssetFile> assets = AssetResolver::parseIndex(assetIndex, versionId);
        for (const AssetFile& asset : assets)
        {
            FileSpec file;
            file.targetRelativePath = asset.relativePath;
            file.urls = MirrorResolver::assetUrls(asset.hash);
            file.size = asset.size;
            file.sha1 = asset.hash.toLatin1();
            plan.files.append(file);

            if (asset.virtualAsset)
            {
                CopyOperation pclCopy;
                pclCopy.sourceRelativePath = asset.relativePath;
                pclCopy.targetRelativePath = QStringLiteral("assets/virtual/legacy/") + asset.legacyPath;
                plan.copies.append(pclCopy);

                CopyOperation standardCopy;
                standardCopy.sourceRelativePath = asset.relativePath;
                standardCopy.targetRelativePath = QStringLiteral("resources/") + asset.legacyPath;
                plan.copies.append(standardCopy);
            }

            if (asset.mapToResources)
            {
                CopyOperation copy;
                copy.sourceRelativePath = asset.relativePath;
                copy.targetRelativePath = QStringLiteral("versions/%1/resources/%2").arg(versionId, asset.legacyPath);
                plan.copies.append(copy);
            }
        }
    }

    const QJsonObject loggingFile = versionJson.value(QStringLiteral("logging")).toObject()
                                        .value(QStringLiteral("client")).toObject()
                                        .value(QStringLiteral("file")).toObject();
    if (!loggingFile.isEmpty())
    {
        const QString id = loggingFile.value(QStringLiteral("id")).toString();
        if (!id.isEmpty())
        {
            const QString target = QStringLiteral("assets/log_configs/%1").arg(id);
            plan.files.append(makeDownload(target, loggingFile, true));
        }
    }

    return plan;
}
