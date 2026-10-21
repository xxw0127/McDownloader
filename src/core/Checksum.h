#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <QByteArray>
#include <QString>

class Checksum
{
public:
    static QByteArray sha1File(const QString& path, QString* error);
    static bool verifyFile(const QString& path, qint64 expectedSize, const QByteArray& expectedSha1, QString* error);
};

#endif // CHECKSUM_H
