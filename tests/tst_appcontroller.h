#ifndef TST_APPCONTROLLER_H
#define TST_APPCONTROLLER_H

#include <QtTest>

class TstAppController : public QObject
{
    Q_OBJECT

private slots:
    void folderValidation();
    void instanceIdForVanilla();
    void startWithoutVersionEmitsError();
    void settingsRoundTrip();
    void startValidRequestStartsWorker();
};

#endif // TST_APPCONTROLLER_H
