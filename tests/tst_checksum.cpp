#include "tst_checksum.h"

#include "core/Checksum.h"

void TstChecksum::sha1Matches()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("hello");
    file.flush();

    QString error;
    const QByteArray digest = Checksum::sha1File(file.fileName(), &error);
    QVERIFY(error.isEmpty());
    QCOMPARE(digest.toHex(), QByteArray("aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d"));
}

void TstChecksum::verifyRejectsWrongSize()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("hello");
    file.flush();

    QString error;
    QVERIFY(!Checksum::verifyFile(file.fileName(), 999, QByteArray(), &error));
    QVERIFY(!error.isEmpty());
}

void TstChecksum::verifyRejectsWrongHash()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    file.write("hello");
    file.flush();

    QString error;
    QVERIFY(!Checksum::verifyFile(file.fileName(), -1, QByteArray("0000"), &error));
    QVERIFY(!error.isEmpty());
}

void TstChecksum::missingFileReportsError()
{
    QString error;
    QVERIFY(Checksum::sha1File(QStringLiteral("Z:/definitely/not/here.bin"), &error).isEmpty());
    QVERIFY(!error.isEmpty());
}
