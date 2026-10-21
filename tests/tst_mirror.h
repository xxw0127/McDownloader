#ifndef TST_MIRROR_H
#define TST_MIRROR_H

#include <QtTest>

class TstMirror : public QObject
{
    Q_OBJECT

private slots:
    void manifestUrlsOrder();
    void assetUrlsOrder();
    void metaUrlMirrorsPistonMeta();
    void libraryUrlsPreferBmclapi();
    void quiltLibraryUrlsPreferOfficial();
};

#endif // TST_MIRROR_H
