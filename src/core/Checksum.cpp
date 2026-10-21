#include "core/Checksum.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

QByteArray Checksum::sha1File(const QString& path, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
            *error = QStringLiteral("无法打开文件：%1").arg(path);
        return QByteArray();
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    const qint64 blockSize = 1024 * 1024;
    while (!file.atEnd())
    {
        const QByteArray block = file.read(blockSize);
        if (block.isEmpty())
            break;
        hash.addData(block);
    }

    if (file.error() != QFile::NoError)
    {
        if (error != nullptr)
            *error = QStringLiteral("读取文件失败：%1").arg(path);
        return QByteArray();
    }

    if (error != nullptr)
        error->clear();
    return hash.result();
}

bool Checksum::verifyFile(const QString& path, qint64 expectedSize, const QByteArray& expectedSha1, QString* error)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile())
    {
        if (error != nullptr)
            *error = QStringLiteral("文件不存在：%1").arg(path);
        return false;
    }

    if (expectedSize >= 0 && info.size() != expectedSize)
    {
        if (error != nullptr)
            *error = QStringLiteral("文件大小不符：%1（期望 %2，实际 %3）").arg(path).arg(expectedSize).arg(info.size());
        return false;
    }

    if (!expectedSha1.isEmpty())
    {
        QString hashError;
        const QByteArray digest = sha1File(path, &hashError);
        if (digest.isEmpty())
        {
            if (error != nullptr)
                *error = hashError;
            return false;
        }

        bool matched = false;
        if (expectedSha1.size() == digest.size())
            matched = expectedSha1 == digest;
        else
            matched = expectedSha1.toLower() == digest.toHex();

        if (!matched)
        {
            if (error != nullptr)
                *error = QStringLiteral("文件校验失败：%1").arg(path);
            return false;
        }
    }

    if (error != nullptr)
        error->clear();
    return true;
}
