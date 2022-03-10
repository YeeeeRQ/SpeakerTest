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

set(PROJ_SOURCE_FILE
    ./font/SourceHanSansK-Regular.ttf
    res.qrc
)

set(PROJ_UI_DIR ./UI)
set(PROJ_UI_FILE
    ${PROJ_UI_DIR}/Setup4AutoTest.h
    ${PROJ_UI_DIR}/Setup4AutoTest.cpp
    ${PROJ_UI_DIR}/Setup4AutoTest.ui
    ${PROJ_UI_DIR}/setup4mic.h
    ${PROJ_UI_DIR}/setup4mic.cpp
    ${PROJ_UI_DIR}/setup4mic.ui
    ${PROJ_UI_DIR}/qmydisplaydevice.cpp
    ${PROJ_UI_DIR}/qmydisplaydevice.h
    ${PROJ_UI_DIR}/Setup4Devices.h
    ${PROJ_UI_DIR}/Setup4Devices.cpp
    ${PROJ_UI_DIR}/Setup4Devices.ui
    ${PROJ_UI_DIR}/setup4model.h
    ${PROJ_UI_DIR}/setup4model.cpp
    ${PROJ_UI_DIR}/setup4model.ui
    ${PROJ_UI_DIR}/showinfo4result.h
    ${PROJ_UI_DIR}/showinfo4result.cpp
    ${PROJ_UI_DIR}/showinfo4result.ui
    ${PROJ_UI_DIR}/TempForm.ui
)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui

        ./devices/AutoLine.h ./devices/AutoLine.cpp
        ./devices/CodeReader.cpp ./devices/CodeReader.h

        ./config/config.cpp ./config/config.h

        ./log/simplelog.cpp ./log/simplelog.h

        ./util/MyCommonFun.cpp ./util/MyCommonFun.h

        ./RecordWorker/RecordWorker.h ./RecordWorker/RecordWorker.cpp
        ./RecordWorker/DataSource.h ./RecordWorker/DataSource.cpp
        ./RecordWorker/get_aubio_filter.h
        ./RecordWorker/get_aubio_filter.cpp
        ./RecordWorker/AudioProcess.h
        ./RecordWorker/AudioProcess.cpp

        ${PROJ_SOURCE_FILE}
        ${PROJ_UI_FILE}
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

    # 开关 console
#    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SpeakersTest)
endif()
