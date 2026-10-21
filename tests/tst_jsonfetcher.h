#ifndef TST_JSONFETCHER_H
#define TST_JSONFETCHER_H

#include <QtTest>

class TstJsonFetcher : public QObject
{
    Q_OBJECT

private slots:
    void fallsBackToSecondJsonUrl();
    void timeoutEmitsFailed();
    void fetchesRealManifestWhenEnabled();
};

#endif // TST_JSONFETCHER_H
