QT += core network concurrent
CONFIG += c++17
INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/core/Paths.h \
    $$PWD/core/Checksum.h \
    $$PWD/core/Logger.h \
    $$PWD/core/LoaderKind.h \
    $$PWD/core/VersionInfo.h \
    $$PWD/core/VersionManifest.h \
    $$PWD/core/VersionGrouping.h \
    $$PWD/platform/PlatformPaths.h

SOURCES += \
    $$PWD/core/Paths.cpp \
    $$PWD/core/Checksum.cpp \
    $$PWD/core/Logger.cpp \
    $$PWD/core/LoaderKind.cpp \
    $$PWD/core/VersionManifest.cpp \
    $$PWD/core/VersionGrouping.cpp \
    $$PWD/platform/PlatformPaths.cpp
