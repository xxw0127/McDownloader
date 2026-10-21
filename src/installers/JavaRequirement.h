#ifndef JAVAREQUIREMENT_H
#define JAVAREQUIREMENT_H

#include <QJsonObject>
#include <QString>

class JavaRequirement
{
public:
    static int majorForVersion(const QJsonObject& versionJson, const QString& mcVersion);
};

#endif // JAVAREQUIREMENT_H
