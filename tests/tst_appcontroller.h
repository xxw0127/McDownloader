#ifndef TST_APPCONTROLLER_H
#define TST_APPCONTROLLER_H

#include <QtTest>

class TstAppController : public QObject
{
    Q_OBJECT

private slots:
    void folderValidation();
    void instanceIdForVanilla();
    void instanceIdForFabric();
    void startWithoutVersionEmitsError();
    void loaderKindChangeClearsVersion();
    void settingsRoundTrip();
};

#endif // TST_APPCONTROLLER_H
