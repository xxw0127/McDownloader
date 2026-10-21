#include "tst_staging.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "installers/StagingArea.h"

namespace
{
void writeFile(const QString& path, const QByteArray& data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(data);
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return file.readAll();
}
}

void TstStaging::createsUnderTargetAndMerges()
{
    QTemporaryDir target;
    QVERIFY(target.isValid());

    QString error;
    StagingArea staging = StagingArea::create(target.path(), &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QVERIFY(staging.path().startsWith(QDir(target.path()).absolutePath()));
    QVERIFY(staging.minecraftPath().endsWith(QStringLiteral(".minecraft")));

    writeFile(QDir(staging.minecraftPath()).filePath(QStringLiteral("versions/test/test.json")), QByteArrayLiteral("{\"id\":\"test\"}"));
    QVERIFY(staging.mergeInto(&error));
    QVERIFY(QFile::exists(QDir(target.path()).filePath(QStringLiteral("versions/test/test.json"))));

    staging.cleanup();
    QVERIFY(!QFileInfo::exists(staging.path()));
}

void TstStaging::backsUpExistingVersion()
{
    QTemporaryDir target;
    QVERIFY(target.isValid());

    const QString targetJson = QDir(target.path()).filePath(QStringLiteral("versions/test/test.json"));
    writeFile(targetJson, QByteArrayLiteral("old"));

    QString error;
    StagingArea staging = StagingArea::create(target.path(), &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    writeFile(QDir(staging.minecraftPath()).filePath(QStringLiteral("versions/test/test.json")), QByteArrayLiteral("new"));
    QVERIFY(staging.mergeInto(&error));

    QCOMPARE(readFile(targetJson), QByteArrayLiteral("new"));

    const QDir backupDir(QDir(target.path()).filePath(QStringLiteral(".mcdownloader-backup")));
    const QStringList backups = backupDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QCOMPARE(backups.size(), 1);
    QCOMPARE(readFile(backupDir.filePath(backups.first() + QStringLiteral("/test.json"))), QByteArrayLiteral("old"));
    staging.cleanup();
}

void TstStaging::mergesLibraries()
{
    QTemporaryDir target;
    QVERIFY(target.isValid());

    QString error;
    StagingArea staging = StagingArea::create(target.path(), &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    writeFile(QDir(staging.minecraftPath()).filePath(QStringLiteral("libraries/foo/bar.jar")), QByteArrayLiteral("jar"));
    QVERIFY(staging.mergeInto(&error));
    QVERIFY(QFile::exists(QDir(target.path()).filePath(QStringLiteral("libraries/foo/bar.jar"))));
    staging.cleanup();
}

void TstStaging::rollbackRestoresVersionOnLibraryFailure()
{
    QTemporaryDir target;
    QVERIFY(target.isValid());

    writeFile(QDir(target.path()).filePath(QStringLiteral("versions/test/test.json")), QByteArrayLiteral("old"));
    writeFile(QDir(target.path()).filePath(QStringLiteral("libraries")), QByteArrayLiteral("not-a-directory"));

    QString error;
    StagingArea staging = StagingArea::create(target.path(), &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    writeFile(QDir(staging.minecraftPath()).filePath(QStringLiteral("versions/test/test.json")), QByteArrayLiteral("new"));
    writeFile(QDir(staging.minecraftPath()).filePath(QStringLiteral("libraries/foo/bar.jar")), QByteArrayLiteral("jar"));

    QVERIFY(!staging.mergeInto(&error));
    QCOMPARE(readFile(QDir(target.path()).filePath(QStringLiteral("versions/test/test.json"))), QByteArrayLiteral("old"));
    staging.cleanup();
}

void TstStaging::cleanupRemovesStaging()
{
    QTemporaryDir target;
    QVERIFY(target.isValid());

    QString error;
    StagingArea staging = StagingArea::create(target.path(), &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    const QString path = staging.path();
    staging.cleanup();
    QVERIFY(!QFileInfo::exists(path));
}
