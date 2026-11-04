#ifndef TST_PATHS_H
#define TST_PATHS_H

#include <QtTest>

class TstPaths : public QObject
{
    Q_OBJECT

private slots:
    void safeRelativePaths();
    void sanitizeInstanceNames();
    void safeVersionIds();
    void isWithinRoot();
};

#endif // TST_PATHS_H
