INCLUDEPATH += \
    $$PWD/ \
    $$PWD/repo/Source/ \
    $$PWD/repo/Source/igtlutil/ \

# Consuming the library
!equals(TARGET, openigtlink) {
    DEPENDPATH += $$PWD
    LIBS += -lopenigtlink

    win32: LIBS += -lws2_32

    # This mess is what Qt Creator auto-generates when adding an internal
    # library to a project. This is the only remaining possible way to reliably
    # do this; the `link_prl` CONFIG option that is supposed to automate this
    # requires just as much manual configuration for correctly pointing to the
    # Debug and Release directories in Visual Studio builds.
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD/release/
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/debug/
    else:unix: LIBS += -L$$PWD/
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/release/libopenigtlink.a
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/debug/libopenigtlink.a
    else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/release/openigtlink.lib
    else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/debug/openigtlink.lib
    else:unix: PRE_TARGETDEPS += $$PWD/libopenigtlink.a
}
