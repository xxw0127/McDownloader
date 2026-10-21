#ifndef VANILLAINSTALLVALIDATOR_H
#define VANILLAINSTALLVALIDATOR_H

#include <QString>

class VanillaInstallValidator
{
public:
    static QString validate(const QString& minecraftDir, const QString& versionId);
};

#endif // VANILLAINSTALLVALIDATOR_H
