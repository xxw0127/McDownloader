#ifndef INSTALLPLAN_H
#define INSTALLPLAN_H

#include <QString>
#include <QVector>

#include "download/FileSpec.h"

struct CopyOperation
{
    QString sourceRelativePath;
    QString targetRelativePath;
};

struct InstallPlan
{
    QString versionId;
    QString versionJsonPath;
    QString jarPath;
    QString assetIndexId;
    QVector<FileSpec> files;
    QVector<CopyOperation> copies;
};

#endif // INSTALLPLAN_H
