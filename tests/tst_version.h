#ifndef TST_VERSION_H
#define TST_VERSION_H

#include <QtTest>

#include "core/VersionInfo.h"

class TstVersion : public QObject
{
    Q_OBJECT

private slots:
    void parsesManifest();
    void classifiesPclGroups();
    void classifiesAprilFoolsByUtcPlusTwo();
    void latestCardIncludesNewerSnapshot();
    void latestCardSkipsOlderSnapshot();
    void mergePrefersNewerMetadata();
    void mergeDeduplicatesAndPrefersSecondaryOnTie();
    void cacheRoundTrip();
    void cacheRejectsCorruptFile();

private:
    QVector<VersionInfo> parseFixture() const;
    VersionInfo find(const QVector<VersionInfo>& versions, const QString& id) const;
};

#endif // TST_VERSION_H
