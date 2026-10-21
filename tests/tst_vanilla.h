#ifndef TST_VANILLA_H
#define TST_VANILLA_H

#include <QtTest>

class TstVanilla : public QObject
{
    Q_OBJECT

private slots:
    void rulesDefaultDeny();
    void windowsNativesSelected();
    void mavenFallbackPath();
    void assetObjectAndLegacyPaths();
    void loggingConfigIncluded();
    void clientAndAssetIndexIncluded();
    void legacyWithoutClientDoesNotCrash();
};

#endif // TST_VANILLA_H
