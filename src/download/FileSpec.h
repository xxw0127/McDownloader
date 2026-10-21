#ifndef FILESPEC_H
#define FILESPEC_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

struct FileSpec
{
    QString targetRelativePath;
    QList<QUrl> urls;
    qint64 size = -1;
    QByteArray sha1;
    bool required = true;
};

#endif // FILESPEC_H
