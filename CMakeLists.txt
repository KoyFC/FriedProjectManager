cmake_minimum_required(VERSION 3.20)

project(FriedProjectManager LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Turns on AUTOMOC, which the Q_OBJECT macro needs, and sets the C++ standard.
qt_standard_project_setup()

qt_add_executable(fried_project_manager
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
)

target_link_libraries(fried_project_manager PRIVATE Qt6::Widgets)
