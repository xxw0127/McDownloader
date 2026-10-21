#include "installers/LegacyForgeExtractor.h"

#include <QJsonDocument>
#include <QJsonValue>

#include "installers/ZipExtractor.h"

namespace
{
QString mavenDirectory(const QString& coordinate)
{
    const QStringList parts = coordinate.split(QLatin1Char(':'));
    if (parts.size() < 3)
        return QString();

    QString group = parts.at(0);
    group.replace(QLatin1Char('.'), QLatin1Char('/'));
    return group + QLatin1Char('/') + parts.at(1) + QLatin1Char('/') + parts.at(2);
}
}

LegacyForgeProfile LegacyForgeExtractor::parse(const QString& installerJar, const QString& mcVersion, QString* error)
{
    LegacyForgeProfile profile;
    ZipExtractor zip(installerJar);
    if (!zip.isValid())
    {
        *error = QStringLiteral("无法打开 Forge 安装器：%1").arg(installerJar);
        return profile;
    }

    QString readError;
    const QByteArray installProfileRaw = zip.readFile(QStringLiteral("install_profile.json"), &readError);
    if (!readError.isEmpty())
    {
        *error = readError;
        return profile;
    }

    const QJsonDocument installDocument = QJsonDocument::fromJson(installProfileRaw);
    if (!installDocument.isObject())
    {
        *error = QStringLiteral("install_profile.json 无效");
        return profile;
    }

    const QJsonObject installerProfile = installDocument.object();
    const QJsonObject install = installerProfile.value(QStringLiteral("install")).toObject();
    const QJsonObject versionInfo = installerProfile.value(QStringLiteral("versionInfo")).toObject();

    if (!versionInfo.isEmpty())
    {
        profile.versionJson = versionInfo;
        profile.installerFileEntry = install.value(QStringLiteral("filePath")).toString();
        const QString coordinate = install.value(QStringLiteral("path")).toString();
        const QString directory = mavenDirectory(coordinate);
        if (!directory.isEmpty() && !profile.installerFileEntry.isEmpty())
            profile.installerLibraryPath = directory + QLatin1Char('/') + profile.installerFileEntry;
    }
    else
    {
        QString jsonEntry = installerProfile.value(QStringLiteral("json")).toString();
        while (jsonEntry.startsWith(QLatin1Char('/')))
            jsonEntry.remove(0, 1);
        if (jsonEntry.isEmpty())
        {
            *error = QStringLiteral("install_profile.json 没有 versionInfo 或 json");
            return profile;
        }

        const QByteArray versionRaw = zip.readFile(jsonEntry, &readError);
        if (!readError.isEmpty())
        {
            *error = readError;
            return profile;
        }

        const QJsonDocument versionDocument = QJsonDocument::fromJson(versionRaw);
        if (!versionDocument.isObject())
        {
            *error = QStringLiteral("Forge version.json 无效");
            return profile;
        }
        profile.versionJson = versionDocument.object();
    }

    profile.versionId = profile.versionJson.value(QStringLiteral("id")).toString();
    profile.baseVersion = profile.versionJson.value(QStringLiteral("inheritsFrom")).toString();
    if (profile.baseVersion.isEmpty())
        profile.baseVersion = mcVersion;

    if (profile.versionId.isEmpty() || profile.baseVersion.isEmpty())
    {
        *error = QStringLiteral("Forge 版本 JSON 缺少 id 或 inheritsFrom");
        return profile;
    }

    profile.versionJson.insert(QStringLiteral("id"), profile.versionId);
    profile.versionJson.insert(QStringLiteral("inheritsFrom"), profile.baseVersion);

    for (const QString& entry : zip.entries())
    {
        if (entry.startsWith(QStringLiteral("maven/")))
        {
            profile.mavenPrefix = QStringLiteral("maven/");
            break;
        }
    }

    profile.valid = true;
    if (error != nullptr)
        error->clear();
    return profile;
}
