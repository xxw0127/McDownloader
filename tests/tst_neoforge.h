#ifndef TST_NEOFORGE_H
#define TST_NEOFORGE_H

#include <QtTest>

class TstNeoForge : public QObject
{
    Q_OBJECT

private slots:
    void supportMatrix();
    void artifactKinds();
    void instanceIds();
    void parsesMavenVersions1201();
    void parsesMavenVersions1211();
    void installerUrls();
};

#endif // TST_NEOFORGE_H
