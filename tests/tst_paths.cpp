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
}

void TstPaths::sanitizeInstanceNames()
{
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("1.20.1/Fabric:test")),
             QStringLiteral("1.20.1_Fabric_test"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("1.20.1 ")), QStringLiteral("1.20.1"));
    QCOMPARE(Paths::sanitizeInstanceName(QStringLiteral("...")), QStringLiteral("instance"));
}

void TstPaths::isWithinRoot()
{
    QString root = QDir::tempPath();
    QVERIFY(Paths::isWithin(root, root + QStringLiteral("/versions/1.20.1")));
    QVERIFY(!Paths::isWithin(root, QStringLiteral("C:/other/versions")));
}
