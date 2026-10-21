#ifndef FORGEINSTALLER_H
#define FORGEINSTALLER_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrl>

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
                                            const QJsonObject& versionJson);
};

#endif // FORGEINSTALLER_H
