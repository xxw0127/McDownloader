#ifndef MANIFESTCACHE_H
#define MANIFESTCACHE_H

#include <QByteArray>
#include <QString>

class ManifestCache
{
public:
    static QString defaultPath();
    static bool save(const QByteArray& body, QString* error = nullptr);
    static bool load(QByteArray* body, QString* error = nullptr);
    static bool saveTo(const QString& path, const QByteArray& body, QString* error = nullptr);
    static bool loadFrom(const QString& path, QByteArray* body, QString* error = nullptr);
};

#endif // MANIFESTCACHE_H
