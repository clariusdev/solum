TARGET = oem
TEMPLATE = app
QT += core widgets gui bluetooth
CONFIG += c++17 precompile_header
DEFINES += QT_DEPRECATED_WARNINGS
PRECOMPILED_HEADER = pch.h

# ensure to unpack the appropriate libs from the zip file into this folder
LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -loem

SOURCES += main.cpp oemqt.cpp ble.cpp display.cpp pch.cpp
HEADERS += oemqt.h ble.h display.h
FORMS += oemqt.ui
