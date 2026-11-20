# Installing

## Releases

Each release on [GitHub](https://github.com/KoyFC/FriedProjectManager/releases) has a build for Linux and one for Windows. On Linux there are two AppImages: `FriedProjectManager-<version>-x86_64-qt-bundle.AppImage` carries Qt, and `FriedProjectManager-<version>-x86_64-system-qt.AppImage` uses the Qt 6 of the system. On Windows, `FriedProjectManager-<version>-windows-x64.zip` holds `fried_project_manager.exe` with the Qt libraries beside it. Make an AppImage executable (`chmod +x`) to run it.

## Building from source

### Requirements

- CMake 3.20+
- A C++17 compiler
- Qt 6.3+ with Qt Widgets, and its CMake package config available to `find_package(Qt6 ... )` (e.g. the distro's `qt6-qtbase-devel` package)

### Building

[FriedCore](https://github.com/KoyFC/FriedCore), the library the tool shares with Fried Editor, is a submodule in `third_party/fried_core`, so clone with `--recurse-submodules`, or run `git submodule update --init` in a clone made without it.

```sh
cmake -S . -B build
cmake --build build
./build/fried_project_manager
```
