#ifndef ASSETRESOLVER_H
#define ASSETRESOLVER_H

#include <QJsonObject>
#include <QString>
#include <QVector>

struct AssetFile
{
    QString relativePath;
    QString hash;
    qint64 size = -1;
    QString legacyPath;
    bool virtualAsset = false;
    bool mapToResources = false;
};

class AssetResolver
{
public:
    static QVector<AssetFile> parseIndex(const QJsonObject& indexJson, const QString& versionId);
};

#endif // ASSETRESOLVER_H
