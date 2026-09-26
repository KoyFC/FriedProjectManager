# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterwards: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares.

The engine lives in its own repository and is consumed by each game as a pinned Git submodule. This repository holds only the tool.

Written in C++ with [Qt 6](https://www.qt.io/) and [CMake](https://cmake.org/).

## Requirements

- CMake 3.20+
- A C++17 compiler
- Qt 6.3+ with Qt Widgets, and its CMake package config available to `find_package(Qt6 ... )` (e.g. the distro's `qt6-qtbase-devel` package)

## Building

```sh
cmake -S . -B build
cmake --build build
```

Then run it:

```sh
./build/fried_project_manager
```

## Status

It builds and opens its main window. Nothing else is implemented yet.
