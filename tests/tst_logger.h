#ifndef TST_LOGGER_H
#define TST_LOGGER_H

#include <QtTest>

class TstLogger : public QObject
{
    Q_OBJECT

private slots:
    void writesInfoLine();
    void writesErrorLine();
    void writesUtf8Chinese();
};

#endif // TST_LOGGER_H
