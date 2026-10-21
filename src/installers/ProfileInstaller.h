#ifndef PROFILEINSTALLER_H
#define PROFILEINSTALLER_H

#include <QByteArray>
#include <QJsonObject>
#include <QString>

#include "core/LoaderKind.h"
#include "installers/InstallPlan.h"

class ProfileInstaller
{
public:
    static QString instanceId(LoaderKind kind,
                              const QString& mcVersion,
                              const QString& loaderVersion);
    static QJsonObject prepareProfile(const QByteArray& raw, const QString& id, QString* error);
    static InstallPlan buildPlan(const QJsonObject& profile, const QString& mcDir, const QString& id);
};

#endif // PROFILEINSTALLER_H
