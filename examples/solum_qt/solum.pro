TARGET = solum
TEMPLATE = app
QT += core widgets gui bluetooth 3dcore 3drender 3dextras
CONFIG += c++17 precompile_header
DEFINES += QT_DEPRECATED_WARNINGS USE_QT_3D
PRECOMPILED_HEADER = pch.h

# libsolum and its bundled dependencies ship in ../../lib (and ../../lib/libs); no need to copy anything.
LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -lsolum
# Embed an RPATH to ../../lib so the example finds libsolum.so at runtime without LD_LIBRARY_PATH.
QMAKE_RPATHDIR += $$absolute_path($$LIBPATH)

SOURCES += main.cpp solumqt.cpp ble.cpp display.cpp 3d.cpp
HEADERS += solumqt.h ble.h display.h 3d.h
FORMS += solumqt.ui

RESOURCES += \
    solum.qrc
