cmake_minimum_required(VERSION 3.20)

project(FriedProjectManager LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Turns on AUTOMOC, which Q_OBJECT needs, and sets the C++ standard.
qt_standard_project_setup()

qt_add_executable(fried_project_manager
    src/build.h
    src/build.cpp
    src/command_dialog.h
    src/command_dialog.cpp
    src/icon.h
    src/icon.cpp
    src/icon_dialog.h
    src/icon_dialog.cpp
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
    src/new_project_dialog.h
    src/new_project_dialog.cpp
    src/project.h
    src/project.cpp
    src/project_template.h
    src/project_template.cpp
)

qt_add_resources(fried_project_manager "templates"
    PREFIX "/"
    FILES
        templates/project/.gitignore
        templates/project/.vscode/c_cpp_properties.json
        templates/project/.vscode/extensions.json
        templates/project/.vscode/host-cmake.sh
        templates/project/.vscode/host-haxe.sh
        templates/project/.vscode/host-run.sh
        templates/project/.vscode/launch.json
        templates/project/.vscode/settings.json
        templates/project/.vscode/tasks.json
        templates/project/CMakeLists.txt
        templates/project/assets/icon.png
        templates/project/assets/white.png
        templates/project/build.hxml
        templates/project/project.fried
        templates/project/sce_sys/icon0.png
        templates/project/sce_sys/livearea/contents/bg.png
        templates/project/sce_sys/livearea/contents/startup.png
        templates/project/sce_sys/livearea/contents/template.xml
        templates/project/src/Main.hx
)

target_link_libraries(fried_project_manager PRIVATE Qt6::Widgets)
