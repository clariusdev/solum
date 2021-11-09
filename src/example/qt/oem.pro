TARGET = oem
TEMPLATE = app
QT += core widgets gui bluetooth 3dcore 3drender 3dextras
CONFIG += c++17 precompile_header
DEFINES += QT_DEPRECATED_WARNINGS USE_QT_3D
PRECOMPILED_HEADER = pch.h

# ensure to unpack the appropriate libs from the zip file into this folder
LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -loem

SOURCES += main.cpp oemqt.cpp ble.cpp display.cpp 3d.cpp
HEADERS += oemqt.h ble.h display.h 3d.h
FORMS += oemqt.ui

RESOURCES += \
    oem.qrc
