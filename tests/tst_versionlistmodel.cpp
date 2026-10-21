#include "tst_versionlistmodel.h"

#include <QFile>

#include "AppController.h"
#include "core/VersionGrouping.h"
#include "core/VersionManifest.h"
#include "models/VersionListModel.h"

QVector<VersionInfo> TstVersionListModel::fixture() const
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

void TstVersionListModel::collapsedGroupsOnlyShowHeaders()
{
    VersionListModel model;
    model.setVersions(fixture());

    QCOMPARE(model.rowCount(), 4);
    for (int row = 0; row < 4; ++row)
        QCOMPARE(model.data(model.index(row, 0), VersionListModel::HeaderRole).toBool(), true);

    QCOMPARE(model.data(model.index(0, 0), VersionListModel::GroupRole).toInt(),
             static_cast<int>(VersionGroup::Release));
    QCOMPARE(model.data(model.index(0, 0), VersionListModel::GroupNameRole).toString(),
             QStringLiteral("正式版"));
    QCOMPARE(model.data(model.index(0, 0), VersionListModel::CountRole).toInt(), 2);
    QCOMPARE(model.data(model.index(0, 0), VersionListModel::ExpandedRole).toBool(), false);
}

void TstVersionListModel::expandingReleaseAddsRows()
{
    VersionListModel model;
    model.setVersions(fixture());
    model.setGroupExpanded(VersionGroup::Release, true);

    QCOMPARE(model.rowCount(), 4 + 2);
    QCOMPARE(model.data(model.index(0, 0), VersionListModel::ExpandedRole).toBool(), true);
    QCOMPARE(model.data(model.index(1, 0), VersionListModel::IdRole).toString(), QStringLiteral("26.3"));
    QCOMPARE(model.data(model.index(1, 0), VersionListModel::HeaderRole).toBool(), false);
}

void TstVersionListModel::selectionMarksOnlyOneRow()
{
    VersionListModel model;
    model.setVersions(fixture());
    model.setGroupExpanded(VersionGroup::Release, true);

    model.setSelectedId(QStringLiteral("26.3"));
    QCOMPARE(model.data(model.index(1, 0), VersionListModel::SelectedRole).toBool(), true);

    model.setSelectedId(QStringLiteral("1.20.5"));
    QCOMPARE(model.data(model.index(1, 0), VersionListModel::SelectedRole).toBool(), false);
    QCOMPARE(model.data(model.index(2, 0), VersionListModel::SelectedRole).toBool(), true);
}

void TstVersionListModel::emptyManifestStillHasFourHeaders()
{
    VersionListModel model;
    model.setVersions({});

    QCOMPARE(model.rowCount(), 4);
    for (int row = 0; row < 4; ++row)
        QCOMPARE(model.data(model.index(row, 0), VersionListModel::CountRole).toInt(), 0);
}

void TstVersionListModel::appControllerLatestCard()
{
    AppController controller;
    controller.loadMockVersions();

    const QVariantMap release = controller.latestRelease();
    QCOMPARE(release.value(QStringLiteral("id")).toString(), QStringLiteral("26.3"));
    QVERIFY(!release.value(QStringLiteral("releaseTime")).toString().isEmpty());
    QCOMPARE(release.value(QStringLiteral("icon")).toString(), QStringLiteral("qrc:/images/grass.png"));

    const QVariantMap snapshot = controller.latestSnapshot();
    QCOMPARE(snapshot.value(QStringLiteral("id")).toString(), QStringLiteral("26.4-snapshot-5"));
    QCOMPARE(snapshot.value(QStringLiteral("icon")).toString(), QStringLiteral("qrc:/images/command_block.png"));
}
