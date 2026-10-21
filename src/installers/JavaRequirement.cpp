#include "installers/JavaRequirement.h"

#include <QVersionNumber>

int JavaRequirement::majorForVersion(const QJsonObject& versionJson, const QString& mcVersion)
{
    const int explicitMajor = versionJson.value(QStringLiteral("javaVersion")).toObject()
                                  .value(QStringLiteral("majorVersion")).toInt();
    if (explicitMajor > 0)
        return explicitMajor;

    const QVersionNumber version = QVersionNumber::fromString(mcVersion);
    if (version.isNull())
        return 8;

    // 版本 JSON 缺少 javaVersion 时，按 Minecraft 版本给出最低可用 Java。
    if (QVersionNumber::compare(version, QVersionNumber(1, 17)) < 0)
        return 8;
    if (QVersionNumber::compare(version, QVersionNumber(1, 18)) < 0)
        return 16;
    if (QVersionNumber::compare(version, QVersionNumber(1, 20, 5)) < 0)
        return 17;
    return 21;
}
