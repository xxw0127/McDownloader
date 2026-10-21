#ifndef TST_LOADER_H
#define TST_LOADER_H

#include <QtTest>

class TstLoader : public QObject
{
    Q_OBJECT

private slots:
    void parsesFabricIndex();
    void rejectsUnsupportedFabricGame();
    void parsesQuiltIndex();
    void fabricProfileUrlKeepsBuildSuffix();
    void quiltProfileUrlHost();
    void instanceIds();
    void prepareProfileOverridesId();
    void buildPlanUsesMavenUrl();
    void quiltLibraryPrefersOfficial();
};

#endif // TST_LOADER_H
