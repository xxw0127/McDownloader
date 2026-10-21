#ifndef FORGEINSTALLER_H
#define FORGEINSTALLER_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrl>

#include <atomic>

struct ForgeInstallResult
{
    bool ok = false;
    QString versionId;
    QString error;
};

class ForgeInstaller
{
public:
    static bool isLegacy(const QString& mcVersion);
    static QList<QUrl> installerUrls(const QString& mcVersion, const QString& forgeVersion);
    static ForgeInstallResult installModern(const QString& minecraftDir,
                                            const QString& mcVersion,
                                            const QString& forgeVersion,
                                            const QString& javaPath,
                                            const QJsonObject& versionJson,
                                            const std::atomic_bool* cancelFlag = nullptr);
    static ForgeInstallResult installLegacy(const QString& minecraftDir,
                                            const QString& mcVersion,
                                            const QString& forgeVersion);
};

#endif // FORGEINSTALLER_H
