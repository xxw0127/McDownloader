#ifndef TST_STAGING_H
#define TST_STAGING_H

#include <QtTest>

class TstStaging : public QObject
{
    Q_OBJECT

private slots:
    void createsUnderTargetAndMerges();
    void backsUpExistingVersion();
    void mergesLibraries();
    void cleanupRemovesStaging();
    void rollbackRestoresVersionOnLibraryFailure();
};

#endif // TST_STAGING_H
