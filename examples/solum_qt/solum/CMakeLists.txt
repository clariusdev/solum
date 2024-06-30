cmake_minimum_required(VERSION 3.25)

project(solum_qt LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui 3DExtras Bluetooth Widgets)
message("Found Qt? ${Qt6_FOUND}")

find_library(SOLUM_SDK_BINARY solum PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib)
message("Solum SDK binary location: ${SOLUM_SDK_BINARY}")
add_library(SOLUM_SDK UNKNOWN IMPORTED)
set_target_properties(SOLUM_SDK PROPERTIES
    IMPORTED_LOCATION ${SOLUM_SDK_BINARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../../include
)

find_package(OpenIGTLink REQUIRED)

qt_add_executable(solum_qt
    main.cpp solumqt.cpp ble.cpp display.cpp 3d.cpp jobbutton.cpp openigtlink.cpp
    solumqt.h ble.h display.h 3d.h image.h jobbutton.h openigtlink.h
    solum.qrc
    solumqt.ui
)

set_target_properties(solum_qt PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../macos/Info.plist
    MACOSX_BUNDLE_GUI_IDENTIFIER "me.clarius.sdk.solum_qt"
)

target_precompile_headers(solum_qt PRIVATE pch.h)

target_include_directories(solum_qt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(solum_qt PRIVATE
    Qt::Core
    Qt::Gui
    Qt::3DExtras
    Qt::Bluetooth
    Qt::Widgets
    SOLUM_SDK
    OpenIGTLink
)
