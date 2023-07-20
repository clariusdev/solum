TARGET = solum
TEMPLATE = app
QT += core widgets gui bluetooth 3dcore 3drender 3dextras
CONFIG += c++20 precompile_header
DEFINES += QT_DEPRECATED_WARNINGS USE_QT_3D
PRECOMPILED_HEADER = pch.h

CONFIG(release, debug|release): CONFIG += ltcg

include($$PWD/../../../openigtlink/openigtlink.pri)

# ensure to unpack the appropriate libs from the zip file into this folder
LIBPATH = $$PWD/../../../lib
INCLUDEPATH += $$PWD/../../../include
LIBS += -L$$LIBPATH/ -lsolum

SOURCES += main.cpp solumqt.cpp ble.cpp display.cpp 3d.cpp jobbutton.cpp openigtlink.cpp
HEADERS += solumqt.h ble.h display.h 3d.h image.h jobbutton.h openigtlink.h
FORMS += solumqt.ui

RESOURCES += \
    solum.qrc
