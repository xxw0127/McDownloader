#include "installers/VanillaInstallValidator.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

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
