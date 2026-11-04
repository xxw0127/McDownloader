#ifndef TST_LEGACYFORGE_H
#define TST_LEGACYFORGE_H

#include <QtTest>

class TstLegacyForge : public QObject
{
    Q_OBJECT

private slots:
    void parses1710Profile();
    void parses1122Profile();
    void extractsMavenJar();
};

#endif // TST_LEGACYFORGE_H
