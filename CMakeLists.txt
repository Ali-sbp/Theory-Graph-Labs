cmake_minimum_required(VERSION 3.16)
project(lab1-2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Try Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets)
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
    distribution.cpp
    graphgenerator.cpp
    graphanalysis.cpp
    shimbel.cpp
    routesearch.cpp
    bicomp.cpp
    dijkstraneg.cpp
    graphwidget.cpp
)

set(HEADERS
    mainwindow.h
    distribution.h
    graphgenerator.h
    graphanalysis.h
    shimbel.h
    routesearch.h
    bicomp.h
    dijkstraneg.h
    graphwidget.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(Qt6_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)
endif()
