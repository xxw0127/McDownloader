#ifndef PLATFORMPATHS_H
#define PLATFORMPATHS_H

#include <QString>

class PlatformPaths
{
public:
    static QString defaultMinecraftDir();
    static QString logDir();
    static QString configPath();
};

#endif // PLATFORMPATHS_H
