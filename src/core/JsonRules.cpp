#include "core/JsonRules.h"

#include <QJsonValue>
#include <QRegularExpression>
#include <QSysInfo>

namespace
{
bool ruleMatches(const QJsonObject& rule, const QString& osName, const QString& arch)
{
    const QJsonObject os = rule.value(QStringLiteral("os")).toObject();
    if (!os.isEmpty())
    {
        const QString name = os.value(QStringLiteral("name")).toString();
        if (!name.isEmpty() && name != QLatin1String("unknown") && name != osName)
            return false;

        if (os.contains(QStringLiteral("arch")))
        {
            const QString ruleArch = os.value(QStringLiteral("arch")).toString().toLower();
            if (ruleArch == QLatin1String("x86") || ruleArch.contains(QLatin1String("arm")))
                return false;
            if (!ruleArch.isEmpty() && ruleArch != arch.toLower())
                return false;
        }

        if (os.contains(QStringLiteral("version")))
        {
            const QString pattern = os.value(QStringLiteral("version")).toString();
            const QRegularExpression expression(pattern);
            if (expression.isValid())
            {
                const QString version = QSysInfo::kernelVersion();
                if (!version.isEmpty() && !expression.match(version).hasMatch())
                    return false;
            }
        }
    }

    const QJsonObject features = rule.value(QStringLiteral("features")).toObject();
    if (features.contains(QStringLiteral("is_demo_user")))
        return false;
    for (auto it = features.begin(); it != features.end(); ++it)
    {
        if (it.key().startsWith(QLatin1String("quick_play")))
            return false;
    }

    return true;
}
}

bool JsonRules::allowed(const QJsonArray& rules, const QString& osName, const QString& arch)
{
    if (rules.isEmpty())
        return true;

    bool result = false;
    for (const QJsonValue& value : rules)
    {
        const QJsonObject rule = value.toObject();
        if (!ruleMatches(rule, osName, arch))
            continue;

        const QString action = rule.value(QStringLiteral("action")).toString();
        if (action == QLatin1String("allow"))
            result = true;
        else if (action == QLatin1String("disallow"))
            result = false;
    }
    return result;
}

bool JsonRules::libraryAllowed(const QJsonObject& library, const QString& osName, const QString& arch)
{
    if (!library.contains(QStringLiteral("rules")))
        return true;
    return allowed(library.value(QStringLiteral("rules")).toArray(), osName, arch);
}
