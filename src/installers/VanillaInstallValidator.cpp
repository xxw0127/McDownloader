#include "installers/VanillaInstallValidator.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "installers/LibraryResolver.h"

QString VanillaInstallValidator::validate(const QString& minecraftDir, const QString& versionId)
{
    const QDir root(minecraftDir);
    const QString versionPath = root.filePath(QStringLiteral("versions/%1/%1.json").arg(versionId));
    QFile versionFile(versionPath);
    if (!versionFile.exists())
        return QStringLiteral("版本 JSON 不存在：%1").arg(versionPath);
    if (!versionFile.open(QIODevice::ReadOnly))
        return QStringLiteral("无法读取版本 JSON：%1").arg(versionPath);

    const QJsonDocument document = QJsonDocument::fromJson(versionFile.readAll());
    if (!document.isObject())
        return QStringLiteral("版本 JSON 无效：%1").arg(versionPath);

    const QJsonObject versionJson = document.object();
    const QJsonObject client = versionJson.value(QStringLiteral("downloads")).toObject()
                                  .value(QStringLiteral("client")).toObject();
    if (!client.isEmpty())
    {
        const QString jarPath = root.filePath(QStringLiteral("versions/%1/%1.jar").arg(versionId));
        if (!QFileInfo::exists(jarPath))
            return QStringLiteral("客户端 jar 不存在：%1").arg(jarPath);
    }

    const QVector<LibraryFile> libraries = LibraryResolver::resolve(versionJson,
                                                                   QStringLiteral("windows"),
                                                                   QStringLiteral("x64"));
    for (const LibraryFile& library : libraries)
    {
        const QString libraryPath = root.filePath(library.relativePath);
        if (!QFileInfo::exists(libraryPath))
            return QStringLiteral("库文件不存在：%1").arg(libraryPath);
    }

    const QJsonObject assetIndex = versionJson.value(QStringLiteral("assetIndex")).toObject();
    if (!assetIndex.isEmpty())
    {
        const QString indexId = assetIndex.value(QStringLiteral("id")).toString();
        if (!indexId.isEmpty() && indexId != QLatin1String("legacy"))
        {
            const QString indexPath = root.filePath(QStringLiteral("assets/indexes/%1.json").arg(indexId));
            if (!QFileInfo::exists(indexPath))
                return QStringLiteral("资源索引不存在：%1").arg(indexPath);
        }
    }

    return QString();
}
