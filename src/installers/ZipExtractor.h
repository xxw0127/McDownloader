#ifndef ZIPEXTRACTOR_H
#define ZIPEXTRACTOR_H

#include <QByteArray>
#include <QString>
#include <QStringList>

#include <miniz.h>

class ZipExtractor
{
public:
    explicit ZipExtractor(const QString& zipPath);
    ~ZipExtractor();

    bool isValid() const;
    QStringList entries() const;
    QByteArray readFile(const QString& entry, QString* error) const;
    bool extractFile(const QString& entry, const QString& target, QString* error) const;
    bool extractPrefix(const QString& prefix, const QString& targetRoot, QString* error) const;

private:
    mutable mz_zip_archive m_archive;
    bool m_valid = false;
};

#endif // ZIPEXTRACTOR_H
