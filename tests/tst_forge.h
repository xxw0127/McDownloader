#ifndef TST_FORGE_H
#define TST_FORGE_H

#include <QtTest>

class TstForge : public QObject
{
    Q_OBJECT

private slots:
    void legacyDetection();
    void installerUrls();
};

#endif // TST_FORGE_H
