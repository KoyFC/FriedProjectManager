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

- **New Project** writes a project directory holding `project.fried`, `build.hxml`, `CMakeLists.txt`, `src/Main.hx`, an example asset and a `.gitignore`, makes it a Git repository with the engine as a submodule, and opens it. Everything is staged, so the first commit is yours.
- Creating one first shows a notice that it runs `git` and clones the engine repository, with a **Don't show this again** toggle. That choice is stored as `newProject/showGitNotice` in the tool's settings (`~/.config/Fried Engine/Fried Project Manager.conf` on Linux), so deleting the key brings the notice back.
- **Open Project** reads an existing `project.fried`, and **Save** writes the edited fields back, keeping any keys the tool does not know about.
- The platform selector hides the fields the chosen platform does not use.

A generated project compiles and runs as it comes out: `haxe build.hxml`, then `cmake -S . -B build && cmake --build build`. Cloning the submodule needs `git` and network access to the engine repository.

Still to come: the Vita `sce_sys/` and `.vscode/` files, building from inside the tool, and the icon action.
