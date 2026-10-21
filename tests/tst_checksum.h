#ifndef TST_CHECKSUM_H
#define TST_CHECKSUM_H

#include <QtTest>

class TstChecksum : public QObject
{
    Q_OBJECT

private slots:
    void sha1Matches();
    void verifyRejectsWrongSize();
    void verifyRejectsWrongHash();
    void missingFileReportsError();
};

#endif // TST_CHECKSUM_H
