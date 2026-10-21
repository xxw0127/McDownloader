#ifndef TST_JAVA_H
#define TST_JAVA_H

#include <QtTest>

class TstJava : public QObject
{
    Q_OBJECT

private slots:
    void parsesLegacy8();
    void parses17();
    void parses21();
    void parses25();
    void rejectsUnknownOutput();
    void selectPrefersHighestSatisfying();
    void selectRejects32Bit();
    void selectReturnsEmptyWhenNoMatch();
    void installerSelectPrefersExactMajor();
    void requirementFromJson();
    void requirementFallbacks();
};

#endif // TST_JAVA_H
