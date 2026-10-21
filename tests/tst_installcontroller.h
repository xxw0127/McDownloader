#ifndef TST_INSTALLCONTROLLER_H
#define TST_INSTALLCONTROLLER_H

#include <QtTest>

class TstInstallController : public QObject
{
    Q_OBJECT

private slots:
    void missingVersionJsonFails();
    void invalidVersionJsonFails();
    void missingClientJarFails();
    void missingAssetIndexFails();
    void completeInstallPasses();
    void rejectsUnsupportedLoader();
};

#endif // TST_INSTALLCONTROLLER_H
