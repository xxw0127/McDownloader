QT += core network concurrent
CONFIG += c++17
INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/../third_party/miniz

HEADERS += \
    $$PWD/core/Paths.h \
    $$PWD/core/Checksum.h \
    $$PWD/core/Logger.h \
    $$PWD/core/LoaderKind.h \
    $$PWD/core/VersionInfo.h \
    $$PWD/core/AppSettings.h \
    $$PWD/core/VersionManifest.h \
    $$PWD/core/VersionGrouping.h \
    $$PWD/models/VersionListModel.h \
    $$PWD/models/LoaderListModel.h \
    $$PWD/download/FileSpec.h \
    $$PWD/download/DownloadService.h \
    $$PWD/download/JsonFetcher.h \
    $$PWD/download/MirrorResolver.h \
    $$PWD/core/JsonRules.h \
    $$PWD/installers/InstallPlan.h \
    $$PWD/installers/LibraryResolver.h \
    $$PWD/installers/AssetResolver.h \
    $$PWD/installers/VanillaPlanBuilder.h \
    $$PWD/installers/VanillaInstallValidator.h \
    $$PWD/installers/InstallController.h \
    $$PWD/installers/LoaderCatalog.h \
    $$PWD/installers/ProfileInstaller.h \
    $$PWD/installers/ProcessRunner.h \
    $$PWD/installers/JavaLocator.h \
    $$PWD/installers/StagingArea.h \
    $$PWD/installers/ForgeInstaller.h \
    $$PWD/installers/ZipExtractor.h \
    $$PWD/installers/LegacyForgeExtractor.h \
    $$PWD/platform/PlatformPaths.h \
    $$PWD/platform/PlatformProcess.h

SOURCES += \
    $$PWD/core/Paths.cpp \
    $$PWD/core/Checksum.cpp \
    $$PWD/core/Logger.cpp \
    $$PWD/core/LoaderKind.cpp \
    $$PWD/core/AppSettings.cpp \
    $$PWD/core/VersionManifest.cpp \
    $$PWD/core/VersionGrouping.cpp \
    $$PWD/models/VersionListModel.cpp \
    $$PWD/models/LoaderListModel.cpp \
    $$PWD/download/DownloadService.cpp \
    $$PWD/download/JsonFetcher.cpp \
    $$PWD/download/MirrorResolver.cpp \
    $$PWD/core/JsonRules.cpp \
    $$PWD/installers/LibraryResolver.cpp \
    $$PWD/installers/AssetResolver.cpp \
    $$PWD/installers/VanillaPlanBuilder.cpp \
    $$PWD/installers/VanillaInstallValidator.cpp \
    $$PWD/installers/InstallController.cpp \
    $$PWD/installers/LoaderCatalog.cpp \
    $$PWD/installers/ProfileInstaller.cpp \
    $$PWD/installers/ProcessRunner.cpp \
    $$PWD/installers/JavaLocator.cpp \
    $$PWD/installers/StagingArea.cpp \
    $$PWD/installers/ForgeInstaller.cpp \
    $$PWD/installers/ZipExtractor.cpp \
    $$PWD/installers/LegacyForgeExtractor.cpp \
    $$PWD/../third_party/miniz/miniz.c \
    $$PWD/../third_party/miniz/miniz_tdef.c \
    $$PWD/../third_party/miniz/miniz_tinfl.c \
    $$PWD/../third_party/miniz/miniz_zip.c \
    $$PWD/platform/PlatformPaths.cpp \
    $$PWD/platform/PlatformProcess.cpp
