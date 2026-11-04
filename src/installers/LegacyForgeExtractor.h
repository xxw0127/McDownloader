#ifndef LEGACYFORGEEXTRACTOR_H
#define LEGACYFORGEEXTRACTOR_H

#include <QJsonObject>
#include <QString>

struct LegacyForgeProfile
{
    bool valid = false;
    QString versionId;
    QString baseVersion;
    QJsonObject versionJson;
    QString installerFileEntry;
    QString installerLibraryPath;
    QString mavenPrefix;
};

class LegacyForgeExtractor
{
public:
    static LegacyForgeProfile parse(const QString& installerJar, const QString& mcVersion, QString* error);
};

#endif // LEGACYFORGEEXTRACTOR_H
