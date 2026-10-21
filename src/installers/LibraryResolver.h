#ifndef LIBRARYRESOLVER_H
#define LIBRARYRESOLVER_H

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVector>

struct LibraryFile
{
    QString relativePath;
    QList<QUrl> urls;
    qint64 size = -1;
    QByteArray sha1;
    bool isNatives = false;
};

class LibraryResolver
{
public:
    static QVector<LibraryFile> resolve(const QJsonObject& versionJson,
                                        const QString& osName,
                                        const QString& arch);
};

#endif // LIBRARYRESOLVER_H
