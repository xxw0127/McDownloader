#ifndef JSONRULES_H
#define JSONRULES_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class JsonRules
{
public:
    static bool allowed(const QJsonArray& rules, const QString& osName, const QString& arch);
    static bool libraryAllowed(const QJsonObject& library, const QString& osName, const QString& arch);
};

#endif // JSONRULES_H
