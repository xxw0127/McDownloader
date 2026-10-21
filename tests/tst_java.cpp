#include "tst_java.h"

#include "installers/JavaLocator.h"

void TstJava::parsesLegacy8()
{
    const JavaInfo info = JavaLocator::parseVersionOutput(
        QStringLiteral("java version \"1.8.0_402\"\nJava(TM) SE Runtime Environment (build 1.8.0_402-b06)\nJava HotSpot(TM) 64-Bit Server VM (build 25.402-b06, mixed mode)"));
    QCOMPARE(info.major, 8);
    QVERIFY(info.is64Bit);
}

void TstJava::parses17()
{
    const JavaInfo info = JavaLocator::parseVersionOutput(
        QStringLiteral("openjdk version \"17.0.8\" 2023-07-18\nOpenJDK Runtime Environment (build 17.0.8+7)\nOpenJDK 64-Bit Server VM (build 17.0.8+7, mixed mode)"));
    QCOMPARE(info.major, 17);
    QVERIFY(info.is64Bit);
}

void TstJava::parses21()
{
    const JavaInfo info = JavaLocator::parseVersionOutput(
        QStringLiteral("openjdk version \"21.0.2\" 2024-01-16\nOpenJDK 64-Bit Server VM"));
    QCOMPARE(info.major, 21);
    QVERIFY(info.is64Bit);
}

void TstJava::parses25()
{
    const JavaInfo info = JavaLocator::parseVersionOutput(
        QStringLiteral("openjdk version \"25\" 2026-09-15\nOpenJDK 64-Bit Server VM"));
    QCOMPARE(info.major, 25);
    QVERIFY(info.is64Bit);
}

void TstJava::rejectsUnknownOutput()
{
    const JavaInfo info = JavaLocator::parseVersionOutput(QStringLiteral("not a java version"));
    QCOMPARE(info.major, 0);
    QVERIFY(!info.is64Bit);
}

void TstJava::selectPrefersHighestSatisfying()
{
    QVector<JavaInfo> candidates;
    candidates.append({QStringLiteral("C:/Java8/bin/java.exe"), 8, true, QString()});
    candidates.append({QStringLiteral("C:/Java17/bin/java.exe"), 17, true, QString()});
    candidates.append({QStringLiteral("C:/Java21/bin/java.exe"), 21, true, QString()});

    const JavaInfo selected = JavaLocator::select(candidates, 17);
    QCOMPARE(selected.major, 21);
}

void TstJava::selectRejects32Bit()
{
    QVector<JavaInfo> candidates;
    candidates.append({QStringLiteral("C:/Java17-32/bin/java.exe"), 17, false, QString()});
    QVERIFY(JavaLocator::select(candidates, 17).path.isEmpty());
}

void TstJava::selectReturnsEmptyWhenNoMatch()
{
    QVector<JavaInfo> candidates;
    candidates.append({QStringLiteral("C:/Java8/bin/java.exe"), 8, true, QString()});
    QVERIFY(JavaLocator::select(candidates, 17).path.isEmpty());
}

void TstJava::installerSelectPrefersExactMajor()
{
    QVector<JavaInfo> candidates;
    candidates.append({QStringLiteral("C:/Java17/bin/java.exe"), 17, true, QString()});
    candidates.append({QStringLiteral("C:/Java21/bin/java.exe"), 21, true, QString()});
    candidates.append({QStringLiteral("C:/Java25/bin/java.exe"), 25, true, QString()});

    QCOMPARE(JavaLocator::selectForInstaller(candidates, 17).major, 17);
}
