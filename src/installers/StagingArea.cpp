#include "installers/StagingArea.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QVector>
#include <QUuid>

namespace
{
bool copyDirectory(const QString& source, const QString& target, QString* error)
{
    QDir sourceDir(source);
    if (!sourceDir.exists())
        return true;

    QDirIterator it(source, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        const QString sourceFile = it.next();
        const QString relative = sourceDir.relativeFilePath(sourceFile);
        const QString targetFile = QDir(target).filePath(relative);
        if (!QDir().mkpath(QFileInfo(targetFile).absolutePath()))
        {
            *error = QStringLiteral("无法创建目录：%1").arg(QFileInfo(targetFile).absolutePath());
            return false;
        }

        const QString tempFile = targetFile + QStringLiteral(".mcdownloader-tmp");
        QFile::remove(tempFile);
        if (!QFile::copy(sourceFile, tempFile))
        {
            *error = QStringLiteral("复制文件失败：%1").arg(targetFile);
            return false;
        }

        QFile::remove(targetFile);
        if (!QFile::rename(tempFile, targetFile))
        {
            QFile::remove(tempFile);
            *error = QStringLiteral("替换文件失败：%1").arg(targetFile);
            return false;
        }
    }
    return true;
}
}

StagingArea::StagingArea(const QString& path, const QString& minecraftPath)
    : m_path(path)
    , m_minecraftPath(minecraftPath)
{
}

StagingArea StagingArea::create(const QString& targetRoot, QString* error)
{
    const QString tempRoot = QDir(targetRoot).filePath(QStringLiteral(".mcdownloader-temp"));
    if (!QDir().mkpath(tempRoot))
    {
        *error = QStringLiteral("无法创建 staging 目录：%1").arg(tempRoot);
        return StagingArea(QString(), QString());
    }

    const QString path = QDir(tempRoot).filePath(QStringLiteral("install-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
    const QString minecraftPath = QDir(path).filePath(QStringLiteral(".minecraft"));
    if (!QDir().mkpath(minecraftPath))
    {
        *error = QStringLiteral("无法创建 staging minecraft 目录：%1").arg(minecraftPath);
        return StagingArea(QString(), QString());
    }

    if (error != nullptr)
        error->clear();
    return StagingArea(path, minecraftPath);
}

QString StagingArea::path() const
{
    return m_path;
}

QString StagingArea::minecraftPath() const
{
    return m_minecraftPath;
}

bool StagingArea::mergeInto(QString* error)
{
    QDir target(QDir(m_path).absolutePath());
    if (!target.cdUp() || !target.cdUp())
    {
        *error = QStringLiteral("无法定位 staging 的目标目录");
        return false;
    }

    struct VersionBackup
    {
        QString target;
        QString backup;
    };
    QVector<VersionBackup> backups;
    QVector<QString> createdTargets;

    auto rollback = [&backups, &createdTargets]() {
        for (const QString& target : createdTargets)
            QDir(target).removeRecursively();
        createdTargets.clear();

        for (int i = backups.size() - 1; i >= 0; --i)
        {
            const VersionBackup& item = backups.at(i);
            if (!QFileInfo::exists(item.backup))
                continue;
            QDir(item.target).removeRecursively();
            QDir().rename(item.backup, item.target);
        }
        backups.clear();
    };

    const QString versionsSource = QDir(m_minecraftPath).filePath(QStringLiteral("versions"));
    QDir versionsDir(versionsSource);
    if (versionsDir.exists())
    {
        const QStringList versionNames = versionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& versionName : versionNames)
        {
            const QString source = versionsDir.filePath(versionName);
            const QString targetVersion = target.filePath(QStringLiteral("versions/%1").arg(versionName));
            const bool targetExisted = QFileInfo::exists(targetVersion);

            if (targetExisted)
            {
                const QString backupRoot = target.filePath(QStringLiteral(".mcdownloader-backup"));
                if (!QDir().mkpath(backupRoot))
                {
                    rollback();
                    *error = QStringLiteral("无法创建备份目录：%1").arg(backupRoot);
                    return false;
                }

                const QString backup = QDir(backupRoot).filePath(
                    QStringLiteral("%1-%2").arg(versionName,
                                                QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"))));
                if (!QDir().rename(targetVersion, backup))
                {
                    rollback();
                    *error = QStringLiteral("无法备份已有版本：%1").arg(targetVersion);
                    return false;
                }
                backups.append({targetVersion, backup});
            }
            else
            {
                createdTargets.append(targetVersion);
            }

            if (!copyDirectory(source, targetVersion, error))
            {
                rollback();
                return false;
            }
        }
    }

    const QString librariesSource = QDir(m_minecraftPath).filePath(QStringLiteral("libraries"));
    if (QDir(librariesSource).exists())
    {
        if (!copyDirectory(librariesSource, target.filePath(QStringLiteral("libraries")), error))
        {
            rollback();
            return false;
        }
    }

    return true;
}

void StagingArea::cleanup()
{
    if (!m_path.isEmpty())
        QDir(m_path).removeRecursively();
}
