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
    void parsesForgeIndexNewestFirst();
    void fabricProfileUrlKeepsBuildSuffix();
    void quiltProfileUrlHost();
    void instanceIds();
    void prepareProfileOverridesId();
    void prepareQuiltProfileAddsLaunchwrapper();
    void buildPlanUsesMavenUrl();
    void quiltLibraryPrefersOfficial();
    void expandsArchPlaceholder();
};

#endif // TST_LOADER_H
