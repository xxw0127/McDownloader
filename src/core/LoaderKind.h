#ifndef LOADERKIND_H
#define LOADERKIND_H

#include <QDateTime>
#include <QString>

enum class LoaderKind
{
    Vanilla,
    Fabric,
    Quilt,
    Forge,
    NeoForge
};

QString loaderKindName(LoaderKind kind);
LoaderKind loaderKindFromName(const QString& name);

struct LoaderVersion
{
    QString version;
    QString stable;
    QDateTime released;
};

#endif // LOADERKIND_H
