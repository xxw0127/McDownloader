#ifndef MIRRORRESOLVER_H
#define MIRRORRESOLVER_H

#include <QList>
#include <QString>
#include <QUrl>

class MirrorResolver
{
public:
    static QList<QUrl> manifestUrls();
    static QList<QUrl> metaUrls(const QUrl& official);
    static QList<QUrl> assetUrls(const QString& hash);
    static QList<QUrl> libraryUrls(const QString& mavenPath, const QUrl& official);

private:
    static QUrl bmclapiMaven(const QString& mavenPath);
    static QString normalizedMavenPath(const QString& mavenPath, const QUrl& official);
};

#endif // MIRRORRESOLVER_H
