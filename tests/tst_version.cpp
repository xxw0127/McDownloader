#include "tst_version.h"

#include <QFile>
#include <QTemporaryDir>

#include "core/ManifestCache.h"

#include <QFile>

#include "core/VersionGrouping.h"
#include "core/VersionManifest.h"

QVector<VersionInfo> TstVersion::parseFixture() const
{
    QFile file(QStringLiteral(TEST_FIXTURE_DIR "/manifest_small.json"));
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QString error;
    const QVector<VersionInfo> versions = VersionManifest::parse(file.readAll(), &error);
    if (!error.isEmpty())
        qWarning() << error;
    return versions;
}

VersionInfo TstVersion::find(const QVector<VersionInfo>& versions, const QString& id) const
{
    for (const VersionInfo& version : versions)
    {
        if (version.id == id)
            return version;
    }
    return VersionInfo();
}

void TstVersion::parsesManifest()
{
    const QVector<VersionInfo> versions = parseFixture();
    QCOMPARE(versions.size(), 7);

    const VersionInfo infinity = find(versions, QStringLiteral("20w14infinite"));
    QCOMPARE(infinity.displayId, QStringLiteral("20w14∞"));
    QVERIFY(infinity.releaseTime.isValid());
    QCOMPARE(infinity.url.host(), QStringLiteral("example.invalid"));
}

void TstVersion::classifiesPclGroups()
{
    const QVector<VersionInfo> versions = parseFixture();
    QCOMPARE(VersionGrouping::classify(find(versions, QStringLiteral("26.3"))), VersionGroup::Release);
    QCOMPARE(VersionGrouping::classify(find(versions, QStringLiteral("26.4-snapshot-5"))), VersionGroup::Snapshot);
    QCOMPARE(VersionGrouping::classify(find(versions, QStringLiteral("1.20.5"))), VersionGroup::Release);
    QCOMPARE(VersionGrouping::classify(find(versions, QStringLiteral("15w14a"))), VersionGroup::AprilFools);
    QCOMPARE(VersionGrouping::classify(find(versions, QStringLiteral("b1.7.3"))), VersionGroup::Old);
}

void TstVersion::classifiesAprilFoolsByUtcPlusTwo()
{
    VersionInfo version;
    version.id = QStringLiteral("2026-03-31-snapshot");
    version.type = QStringLiteral("snapshot");
    version.releaseTime = QDateTime::fromString(QStringLiteral("2026-03-31T22:30:00Z"), Qt::ISODate);

    // UTC+2 后是 2026-04-01 00:30，应归愚人节版
    QCOMPARE(VersionGrouping::classify(version), VersionGroup::AprilFools);
}

void TstVersion::latestCardIncludesNewerSnapshot()
{
    const QVector<VersionInfo> latest = VersionGrouping::latestForCard(parseFixture());
    QCOMPARE(latest.size(), 2);
    QCOMPARE(latest[0].id, QStringLiteral("26.3"));
    QCOMPARE(latest[1].id, QStringLiteral("26.4-snapshot-5"));
}

void TstVersion::latestCardSkipsOlderSnapshot()
{
    QVector<VersionInfo> versions;
    VersionInfo release;
    release.id = QStringLiteral("1.20.1");
    release.type = QStringLiteral("release");
    release.releaseTime = QDateTime::fromString(QStringLiteral("2023-06-12T10:00:00Z"), Qt::ISODate);
    versions.append(release);

    VersionInfo snapshot;
    snapshot.id = QStringLiteral("23w01a");
    snapshot.type = QStringLiteral("snapshot");
    snapshot.releaseTime = QDateTime::fromString(QStringLiteral("2022-01-01T10:00:00Z"), Qt::ISODate);
    versions.append(snapshot);

    const QVector<VersionInfo> latest = VersionGrouping::latestForCard(versions);
    QCOMPARE(latest.size(), 1);
    QCOMPARE(latest[0].id, QStringLiteral("1.20.1"));
}

void TstVersion::mergePrefersNewerMetadata()
{
    VersionInfo oldVersion;
    oldVersion.id = QStringLiteral("1.20.1");
    oldVersion.type = QStringLiteral("release");
    oldVersion.releaseTime = QDateTime::fromString(QStringLiteral("2023-06-01T00:00:00Z"), Qt::ISODate);
    oldVersion.url = QUrl(QStringLiteral("https://old.example/1.20.1.json"));

    VersionInfo newVersion = oldVersion;
    newVersion.releaseTime = QDateTime::fromString(QStringLiteral("2023-06-12T00:00:00Z"), Qt::ISODate);
    newVersion.url = QUrl(QStringLiteral("https://new.example/1.20.1.json"));

    const QVector<VersionInfo> merged = VersionManifest::merge({oldVersion}, {newVersion});
    QCOMPARE(merged.size(), 1);
    QCOMPARE(merged[0].releaseTime, newVersion.releaseTime);
    QCOMPARE(merged[0].url, newVersion.url);
}

void TstVersion::mergeDeduplicatesAndPrefersSecondaryOnTie()
{
    VersionInfo primary;
    primary.id = QStringLiteral("1.20.1");
    primary.type = QStringLiteral("release");
    primary.releaseTime = QDateTime::fromString(QStringLiteral("2023-06-12T00:00:00Z"), Qt::ISODate);
    primary.url = QUrl(QStringLiteral("https://bmclapi.example/1.20.1.json"));

    VersionInfo secondary = primary;
    secondary.url = QUrl(QStringLiteral("https://piston-meta.mojang.com/1.20.1.json"));

    const QVector<VersionInfo> merged = VersionManifest::merge({primary}, {secondary});
    QCOMPARE(merged.size(), 1);
    QCOMPARE(merged[0].url, secondary.url);
}

void TstVersion::cacheRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("version_manifest.json"));
    const QByteArray body = QByteArrayLiteral(
        "{\"versions\":[{\"id\":\"1.20.1\",\"type\":\"release\","
        "\"releaseTime\":\"2023-06-12T13:25:51+00:00\","
        "\"url\":\"https://example/1.20.1.json\"}]}");

    QString error;
    QVERIFY(ManifestCache::saveTo(path, body, &error));
    QVERIFY2(error.isEmpty(), qPrintable(error));

    QByteArray loaded;
    QVERIFY(ManifestCache::loadFrom(path, &loaded, &error));
    QCOMPARE(loaded, body);
}

void TstVersion::cacheRejectsCorruptFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("version_manifest.json"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("<html>not json</html>");
    file.close();

    QByteArray loaded;
    QString error;
    QVERIFY(!ManifestCache::loadFrom(path, &loaded, &error));
    QVERIFY(!error.isEmpty());
}
