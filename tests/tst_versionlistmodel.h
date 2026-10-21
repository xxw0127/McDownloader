#ifndef TST_VERSIONLISTMODEL_H
#define TST_VERSIONLISTMODEL_H

#include <QtTest>

#include "core/VersionInfo.h"

class TstVersionListModel : public QObject
{
    Q_OBJECT

private slots:
    void collapsedGroupsOnlyShowHeaders();
    void expandingReleaseAddsRows();
    void selectionMarksOnlyOneRow();
    void emptyManifestStillHasFourHeaders();
    void appControllerLatestCard();

private:
    QVector<VersionInfo> fixture() const;
};

#endif // TST_VERSIONLISTMODEL_H
