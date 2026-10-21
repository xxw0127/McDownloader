QT += testlib
CONFIG += console testcase c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = tst_main

INCLUDEPATH += $$PWD/../src
DEFINES += TEST_FIXTURE_DIR=\\\"$$PWD/fixtures\\\"

include(../src/core.pri)

HEADERS += \
    $$PWD/tst_paths.h \
    $$PWD/tst_checksum.h \
    $$PWD/tst_logger.h \
    $$PWD/tst_version.h

SOURCES += \
    $$PWD/tst_main.cpp \
    $$PWD/tst_paths.cpp \
    $$PWD/tst_checksum.cpp \
    $$PWD/tst_logger.cpp \
    $$PWD/tst_version.cpp
