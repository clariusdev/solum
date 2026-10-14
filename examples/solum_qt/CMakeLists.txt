cmake_minimum_required(VERSION 3.25)

project(solum_qt LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui 3DExtras Bluetooth Widgets)
message("Found Qt? ${Qt6_FOUND}")

find_library(SOLUM_SDK_BINARY solum PATHS ${CMAKE_SOURCE_DIR}/../../lib)
message("Solum SDK binary location: ${SOLUM_SDK_BINARY}")
add_library(SOLUM_SDK UNKNOWN IMPORTED)
set_target_properties(SOLUM_SDK PROPERTIES
    IMPORTED_LOCATION ${SOLUM_SDK_BINARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/../../include
)

qt_add_executable(solum_qt
    main.cpp solumqt.cpp ble.cpp display.cpp 3d.cpp
    solumqt.h ble.h display.h 3d.h
    solum.qrc
    solumqt.ui
)

set_target_properties(solum_qt PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macos/Info.plist
    MACOSX_BUNDLE_GUI_IDENTIFIER "me.clarius.sdk.solum_qt"
)

target_precompile_headers(solum_qt PRIVATE pch.h)

target_link_libraries(solum_qt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::3DExtras
    Qt::Bluetooth
    Qt::Widgets
    SOLUM_SDK
)

# On Linux, embed an RPATH to the SDK's lib/ so the built example finds libsolum.so (and the deps
# bundled in lib/libs/) at runtime, with no need to set LD_LIBRARY_PATH or copy any .so around.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    get_filename_component(SOLUM_SDK_LIBDIR "${CMAKE_SOURCE_DIR}/../../lib" ABSOLUTE)
    set_target_properties(solum_qt PROPERTIES
        BUILD_RPATH "${SOLUM_SDK_LIBDIR}"
        INSTALL_RPATH "${SOLUM_SDK_LIBDIR}"
    )
endif()
