cmake_minimum_required(VERSION 3.5)

project(SpeakersTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Charts REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS SerialPort REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS AxContainer REQUIRED)

# aubio
set(AUBIO_INC_DIR "..\\aubio-0.4.6-win64\\include")
set(AUBIO_LINK_DIR "..\\aubio-0.4.6-win64\\lib")
include_directories(${AUBIO_INC_DIR})
link_directories(${AUBIO_LINK_DIR})
set(CMAKE_CXX_FLAGS
    "-D_GLIBCXX_USE_CXX11_ABI=0"
    )
set(TS_FILES SpeakersTest_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp

        mainwindow.cpp mainwindow.h mainwindow.ui
        Setup4AutoTest.h Setup4AutoTest.cpp Setup4AutoTest.ui
        setup4mic.h setup4mic.cpp setup4mic.ui
        Dlg4ChangeValue.h Dlg4ChangeValue.cpp Dlg4ChangeValue.ui
        Setup4Devices.h Setup4Devices.cpp Setup4Devices.ui
        setup4model.h setup4model.cpp setup4model.ui
        showinfo4result.h showinfo4result.cpp showinfo4result.ui

        qmydisplaydevice.cpp qmydisplaydevice.h
        RecordWorker.h RecordWorker.cpp
        AutoLine.h AutoLine.cpp
        CodeReader.cpp CodeReader.h
        config.cpp config.h
        simplelog.cpp simplelog.h

        MyCommonFun.cpp MyCommonFun.h
        TempForm.ui
        res.qrc

#        AudioProcess.h AudioProcess.cpp

        ./font/SourceHanSansK-Regular.ttf
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SpeakersTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SpeakersTest APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SpeakersTest SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SpeakersTest
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
#    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()


target_link_libraries(SpeakersTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::AxContainer
    aubio.dll.a
    libaubio.a
)

set_target_properties(SpeakersTest PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SpeakersTest)
endif()
