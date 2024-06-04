TARGET = solum
TEMPLATE = app
CONFIG += c++17 console

# ensure to unpack the appropriate libs from the zip file into this folder
LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -lsolum

SOURCES += main.cpp
