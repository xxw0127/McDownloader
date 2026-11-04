#include "tst_paths.h"

#include "core/Paths.h"

void TstPaths::safeRelativePaths()
{
    QVERIFY(Paths::isSafeRelativePath(QStringLiteral("versions/1.20.1/1.20.1.json")));
    QVERIFY(Paths::isSafeRelativePath(QStringLiteral("assets/objects/ab/abcdef")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("../escape")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("versions/../../escape")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("C:/Windows/x")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("/root/x")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("\\root\\x")));
    QVERIFY(!Paths::isSafeRelativePath(QString()));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("CON/x")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("versions/bad?.json")));
    QVERIFY(!Paths::isSafeRelativePath(QStringLiteral("versions/trailing./x")));
}

void TstPaths::sanitizeInstanceNames()
{
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("1.20.1/Fabric:test")),
             QStringLiteral("1.20.1_Fabric_test"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("1.20.1 ")), QStringLiteral("1.20.1"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("...")), QStringLiteral("instance"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("a\\b")), QStringLiteral("a_b"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("CON")), QStringLiteral("_CON"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("COM1.txt")), QStringLiteral("_COM1.txt"));
}

void TstPaths::safeVersionIds()
{
    QVERIFY(Paths::isSafeVersionId(QStringLiteral("1.20.1")));
    QVERIFY(Paths::isSafeVersionId(QStringLiteral("fabric-loader-0.15.11-1.20.1")));
    QVERIFY(!Paths::isSafeVersionId(QStringLiteral("../escape")));
    QVERIFY(!Paths::isSafeVersionId(QStringLiteral("1.20.1/../../x")));
    QVERIFY(!Paths::isSafeVersionId(QStringLiteral("CON")));
    QVERIFY(!Paths::isSafeVersionId(QString()));
}

void TstPaths::isWithinRoot()
{
    QString root = QDir::tempPath();
    QVERIFY(Paths::isWithin(root, root + QStringLiteral("/versions/1.20.1")));
    QVERIFY(!Paths::isWithin(root, QStringLiteral("C:/other/versions")));
}
