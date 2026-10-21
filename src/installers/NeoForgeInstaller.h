#ifndef NEOFORGEINSTALLER_H
#define NEOFORGEINSTALLER_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVector>

#include "core/LoaderKind.h"

struct NeoForgeInstallResult
{
    bool ok = false;
    QString versionId;
    QString error;
};

class NeoForgeInstaller
{
public:
    static bool isSupported(const QString& mcVersion);
    static QString artifactKind(const QString& mcVersion);
    static QString instanceId(const QString& mcVersion, const QString& loaderVersion);
    static QList<QUrl> installerUrls(const QString& mcVersion, const QString& loaderVersion);
    static QVector<LoaderVersion> parseMavenVersions(const QByteArray& body,
                                                     const QString& mcVersion,
                                                     QString* error);
    static NeoForgeInstallResult install(const QString& minecraftDir,
                                         const QString& mcVersion,
                                         const QString& loaderVersion,
                                         const QString& javaPath,
                                         const QJsonObject& versionJson);
};

#endif // NEOFORGEINSTALLER_H
