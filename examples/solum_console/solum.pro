TARGET = solum
TEMPLATE = app
CONFIG += c++17 console

# libsolum and its bundled dependencies ship in ../../lib (and ../../lib/libs); no need to copy anything.
LIBPATH = $$PWD/../../lib
INCLUDEPATH += $$PWD/../../include
LIBS += -L$$LIBPATH/ -lsolum
# Embed an RPATH to ../../lib so the example finds libsolum.so at runtime without LD_LIBRARY_PATH.
QMAKE_RPATHDIR += $$absolute_path($$LIBPATH)

SOURCES += main.cpp
