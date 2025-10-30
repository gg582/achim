cmake_minimum_required(VERSION 3.21)
project(achim_alarm LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets Multimedia)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/alarmwindow.cpp
    resources/achim.qrc
)

set(HEADERS
    src/alarmwindow.h
)

qt_add_executable(achim_alarm
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(achim_alarm PRIVATE Qt6::Widgets Qt6::Multimedia)

install(TARGETS achim_alarm
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
