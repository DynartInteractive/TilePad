cmake_minimum_required(VERSION 3.16)

project(TilePad VERSION 0.5.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(TilePad
    main.cpp
    mainwindow.h mainwindow.cpp
    pixmapdropwidget.h pixmapdropwidget.cpp
    paddinggenerator.h paddinggenerator.cpp
    paddingremover.h paddingremover.cpp
    coloredit.h coloredit.cpp
    thememanager.h thememanager.cpp
    titlebar.h titlebar.cpp
    resources.qrc
)

target_link_libraries(TilePad PRIVATE Qt6::Widgets)

if(WIN32)
    target_link_libraries(TilePad PRIVATE dwmapi)
    set_target_properties(TilePad PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

install(TARGETS TilePad
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
