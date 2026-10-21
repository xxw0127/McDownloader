#ifndef TST_DOWNLOAD_H
#define TST_DOWNLOAD_H

#include <QtTest>

class TstDownload : public QObject
{
    Q_OBJECT

private slots:
    void downloadsAndVerifies();
    void fallsBackToSecondUrl();
    void rejectsBadSha1();
    void skipsExistingValidFile();
    void cancelLeavesNoFinalFile();
    void rejectsUnsafeRelativePath();
};

#endif // TST_DOWNLOAD_H
