cmake_minimum_required(VERSION 3.5)

project(qfcmd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ASAN "Enable AddressSanitizer" OFF)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools DBus)

set(TS_FILES lang/qfcmd_zh_CN.ts)

set(PROJECT_SOURCES
        # Public header
        include/qfcmd/filesystem.h
        # Entrypoint
        src/main.cpp
        # Widgets
        src/widget/fsfoldertab.hpp
        src/widget/fsfoldertab.cpp
        src/widget/fstabwidget.hpp
        src/widget/fstabwidget.cpp
        src/widget/fstreeview.hpp
        src/widget/fstreeview.cpp
        src/widget/mainwindow.cpp
        src/widget/mainwindow.hpp
        src/widget/perferencesdialog.hpp
        src/widget/perferencesdialog.cpp
        src/widget/perferencesdialog.ui
        # About dialog
        src/aboutdialog.hpp
        src/aboutdialog.cpp
        src/aboutdialog.ui
        # Model
        src/model/filesystem.hpp
        src/model/filesystem.cpp
        # Utils
        src/utils/container.hpp
        src/utils/container.cpp
        src/utils/log.hpp
        src/utils/log.cpp
        src/utils/win32.hpp
        src/utils/win32.cpp
        # Settings
        src/settings.hpp
        src/settings.cpp
        # VFS
        src/vfs/filesystem.hpp
        src/vfs/filesystem.cpp
        src/vfs/local.hpp
        src/vfs/local.cpp
        src/vfs/vfs.hpp
        src/vfs/vfs.cpp
        # Resources
        res/app.qrc
        res/app.rc
        # i18n
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qfcmd
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qfcmd APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    # qt_create_translation() is deprecated. See https://doc.qt.io/qt-6/qtlinguist-cmake-qt-create-translation.html
    find_package(Qt6 REQUIRED COMPONENTS LinguistTools)
    qt_add_translations(${PROJECT_NAME} TS_FILES ${TS_FILES})
else()
    if(ANDROID)
        add_library(qfcmd SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qfcmd
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(qfcmd
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(qfcmd
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
)
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(qfcmd PRIVATE /W4 /WX)
else ()
    target_compile_options(qfcmd PRIVATE -Wall -Wextra -Werror)
endif ()

if (ASAN)
    target_compile_options(qfcmd PRIVATE -fsanitize=address)
	target_link_options(qfcmd PRIVATE -fsanitize=address)
endif ()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qfcmd)
endif()
set_target_properties(qfcmd PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qfcmd
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qfcmd)
endif()
