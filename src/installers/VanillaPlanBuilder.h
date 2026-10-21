#ifndef VANILLAPLANBUILDER_H
#define VANILLAPLANBUILDER_H

#include <QJsonObject>
#include <QString>

#include "installers/InstallPlan.h"

class VanillaPlanBuilder
{
public:
    static InstallPlan build(const QJsonObject& versionJson,
                             const QString& mcDir,
                             const QString& versionId,
                             const QJsonObject& assetIndex = QJsonObject());
};

#endif // VANILLAPLANBUILDER_H
