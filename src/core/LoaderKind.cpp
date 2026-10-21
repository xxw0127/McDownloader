#include "core/LoaderKind.h"

QString loaderKindName(LoaderKind kind)
{
    switch (kind)
    {
    case LoaderKind::Vanilla:
        return QStringLiteral("vanilla");
    case LoaderKind::Fabric:
        return QStringLiteral("fabric");
    case LoaderKind::Quilt:
        return QStringLiteral("quilt");
    case LoaderKind::Forge:
        return QStringLiteral("forge");
    case LoaderKind::NeoForge:
        return QStringLiteral("neoforge");
    }
    return QStringLiteral("vanilla");
}

LoaderKind loaderKindFromName(const QString& name)
{
    const QString key = name.trimmed().toLower();
    if (key == QLatin1String("fabric"))
        return LoaderKind::Fabric;
    if (key == QLatin1String("quilt"))
        return LoaderKind::Quilt;
    if (key == QLatin1String("forge"))
        return LoaderKind::Forge;
    if (key == QLatin1String("neoforge"))
        return LoaderKind::NeoForge;
    return LoaderKind::Vanilla;
}
