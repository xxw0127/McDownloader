#ifndef LOADERCATALOG_H
#define LOADERCATALOG_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVector>

#include "core/LoaderKind.h"

class LoaderCatalog
{
public:
    static QList<QUrl> indexUrls(LoaderKind kind, const QString& mcVersion);
    static QVector<LoaderVersion> parseIndex(LoaderKind kind,
                                             const QByteArray& body,
                                             const QString& mcVersion,
                                             QString* error);
    static QList<QUrl> profileUrls(LoaderKind kind,
                                   const QString& mcVersion,
                                   const QString& loaderVersion);
};

#endif // LOADERCATALOG_H
