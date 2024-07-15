cmake_minimum_required(VERSION 3.10)
project(broadlink_mp1)

set(CMAKE_CXX_STANDARD 14/home/ksh/workspace/broadlink_mp1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_PREFIX_PATH "$ENV{QTDIR}")
set(QT_QMAKE_EXECUTABLE qmake.exe)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (MSVC)
    set(GUI_TYPE WIN32)
    add_definitions(-DWITHOUT_CONSOLE)
endif()

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(OpenSSL)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

set(FORMS
        mainwindow.ui
)
qt5_wrap_ui(FORMS_HEADERS ${FORMS})

set(SOURCES
        main.cpp
        mainwindow.h
        mainwindow.cpp
        BroadLinkMP1.h
        BroadLinkMP1.cpp)

add_executable(broadlink_mp1
        ${GUI_TYPE}
        ${SOURCES}
        ${FORMS_HEADERS}
)
target_link_libraries(broadlink_mp1 Qt5::Widgets Qt5::Network)
target_link_libraries(broadlink_mp1 ${OPENSSL_LIBRARIES})
