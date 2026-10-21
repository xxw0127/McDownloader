#ifndef TST_PROCESS_H
#define TST_PROCESS_H

#include <QtTest>

class TstProcess : public QObject
{
    Q_OBJECT

private slots:
    void capturesExitCode();
    void capturesStdout();
    void timesOut();
    void reportsMissingExecutable();
};

#endif // TST_PROCESS_H
