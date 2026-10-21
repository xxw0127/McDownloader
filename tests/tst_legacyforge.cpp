#include "tst_legacyforge.h"

#include <QFile>
#include <QTemporaryDir>

#include "installers/LegacyForgeExtractor.h"
#include "installers/ZipExtractor.h"

void TstLegacyForge::parses1710Profile()
{
    QString error;
    const LegacyForgeProfile profile = LegacyForgeExtractor::parse(
        QStringLiteral(TEST_FIXTURE_DIR "/legacy_forge_1710_installer.jar"),
        QStringLiteral("1.7.10"), &error);
    QVERIFY2(profile.valid, qPrintable(error));
    QCOMPARE(profile.versionId, QStringLiteral("1.7.10-Forge10.13.4.1614-1.7.10"));
    QCOMPARE(profile.baseVersion, QStringLiteral("1.7.10"));
    QVERIFY(profile.installerFileEntry.contains(QStringLiteral("universal.jar")));
    QVERIFY(profile.installerLibraryPath.contains(QStringLiteral("net/minecraftforge/forge/")));
    QCOMPARE(profile.versionJson.value(QStringLiteral("libraries")).toArray().size(), 1);
}

void TstLegacyForge::parses1122Profile()
{
    QString error;
    const LegacyForgeProfile profile = LegacyForgeExtractor::parse(
        QStringLiteral(TEST_FIXTURE_DIR "/legacy_forge_1122_installer.jar"),
        QStringLiteral("1.12.2"), &error);
    QVERIFY2(profile.valid, qPrintable(error));
    QCOMPARE(profile.versionId, QStringLiteral("1.12.2-forge-14.23.5.2860"));
    QCOMPARE(profile.baseVersion, QStringLiteral("1.12.2"));
    QCOMPARE(profile.mavenPrefix, QStringLiteral("maven/"));
}

void TstLegacyForge::extractsMavenJar()
{
    ZipExtractor zip(QStringLiteral(TEST_FIXTURE_DIR "/legacy_forge_1122_installer.jar"));
    QVERIFY(zip.isValid());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString error;
    QVERIFY2(zip.extractPrefix(QStringLiteral("maven/"), dir.path(), &error), qPrintable(error));
    QVERIFY(QFile::exists(dir.filePath(QStringLiteral("net/minecraftforge/forge/1.12.2-14.23.5.2860/forge-1.12.2-14.23.5.2860.jar"))));
}
