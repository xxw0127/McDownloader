#include "installers/ZipExtractor.h"

#include <QDir>
#include <QFileInfo>

#include <cstring>

#include "core/Paths.h"

namespace
{
QString normalizedEntryName(QString name)
{
    name.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return name;
}

int locateEntry(const mz_zip_archive& archive, const QString& entry)
{
    int index = mz_zip_reader_locate_file(const_cast<mz_zip_archive*>(&archive),
                                          entry.toUtf8().constData(), nullptr, 0);
    if (index >= 0)
        return index;

    QString alternative = entry;
    alternative.replace(QLatin1Char('/'), QLatin1Char('\\'));
    return mz_zip_reader_locate_file(const_cast<mz_zip_archive*>(&archive),
                                     alternative.toUtf8().constData(), nullptr, 0);
}
}

ZipExtractor::ZipExtractor(const QString& zipPath)
{
    std::memset(&m_archive, 0, sizeof(m_archive));
    m_valid = mz_zip_reader_init_file(&m_archive, zipPath.toUtf8().constData(), 0) != 0;
}

ZipExtractor::~ZipExtractor()
{
    if (m_valid)
        mz_zip_reader_end(&m_archive);
}

bool ZipExtractor::isValid() const
{
    return m_valid;
}

QStringList ZipExtractor::entries() const
{
    QStringList result;
    if (!m_valid)
        return result;

    const mz_uint count = mz_zip_reader_get_num_files(&m_archive);
    for (mz_uint i = 0; i < count; ++i)
    {
        const mz_uint size = mz_zip_reader_get_filename(&m_archive, i, nullptr, 0);
        if (size <= 1)
            continue;
        QByteArray buffer(static_cast<int>(size), '\0');
        mz_zip_reader_get_filename(&m_archive, i, buffer.data(), size);
        result.append(normalizedEntryName(QString::fromUtf8(buffer.constData())));
    }
    return result;
}

QByteArray ZipExtractor::readFile(const QString& entry, QString* error) const
{
    if (!m_valid)
    {
        *error = QStringLiteral("ZIP 未打开");
        return {};
    }

    const int index = locateEntry(m_archive, entry);
    if (index < 0)
    {
        *error = QStringLiteral("ZIP 中找不到条目：%1").arg(entry);
        return {};
    }

    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&m_archive, static_cast<mz_uint>(index), &stat))
    {
        *error = QStringLiteral("无法读取 ZIP 条目信息：%1").arg(entry);
        return {};
    }

    const mz_ulong size = stat.m_uncomp_size;
    QByteArray data;
    data.resize(static_cast<int>(size));
    if (!mz_zip_reader_extract_to_mem(&m_archive, static_cast<mz_uint>(index),
                                      data.data(), size, 0))
    {
        *error = QStringLiteral("解压 ZIP 条目失败：%1").arg(entry);
        return {};
    }

    if (error != nullptr)
        error->clear();
    return data;
}

bool ZipExtractor::extractFile(const QString& entry, const QString& target, QString* error) const
{
    const QByteArray data = readFile(entry, error);
    if (error != nullptr && !error->isEmpty())
        return false;

    if (!QDir().mkpath(QFileInfo(target).absolutePath()))
    {
        *error = QStringLiteral("无法创建解压目录：%1").arg(QFileInfo(target).absolutePath());
        return false;
    }

    const int index = locateEntry(m_archive, entry);
    if (index < 0 || !mz_zip_reader_extract_to_file(&m_archive, static_cast<mz_uint>(index),
                                                     target.toUtf8().constData(), 0))
    {
        *error = QStringLiteral("无法写入解压文件：%1").arg(target);
        return false;
    }

    return true;
}

bool ZipExtractor::extractPrefix(const QString& prefix, const QString& targetRoot, QString* error) const
{
    const QStringList names = entries();
    for (const QString& name : names)
    {
        if (!name.startsWith(prefix) || name.endsWith(QLatin1Char('/')))
            continue;

        const QString relative = name.mid(prefix.size());
        if (!Paths::isSafeRelativePath(relative))
        {
            *error = QStringLiteral("ZIP 条目路径不安全：%1").arg(name);
            return false;
        }

        bool ok = false;
        const QString target = Paths::joinWithin(targetRoot, relative, &ok);
        if (!ok)
        {
            *error = QStringLiteral("ZIP 条目路径越界：%1").arg(name);
            return false;
        }

        if (!extractFile(name, target, error))
            return false;
    }
    return true;
}
