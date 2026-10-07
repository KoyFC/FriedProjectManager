# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterwards: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares. It builds one for PC, the PlayStation Vita, the Nintendo Switch and the Nintendo 3DS, all from the same sources.

The engine lives in its own repository and is consumed by each game as a pinned Git submodule. This repository holds only the tool. See [`ARCHITECTURE.md`](ARCHITECTURE.md) for the decisions behind it.

Written in C++ with [Qt 6](https://www.qt.io/) and [CMake](https://cmake.org/).

## Requirements

- CMake 3.20+
- A C++17 compiler
- Qt 6.3+ with Qt Widgets, and its CMake package config available to `find_package(Qt6 ... )` (e.g. the distro's `qt6-qtbase-devel` package)

## Building

```sh
cmake -S . -B build
cmake --build build
./build/fried_project_manager
```

## What it does

- **Home screen** with the last 20 projects opened, most recent first, each with its icon, name and directory. One whose `project.fried` is gone is listed as **(not found)**, and **Locate...** points it at where it moved to. **Remove** takes an entry off the list without deleting anything.
- **New Project** writes a whole project directory, makes it a Git repository with the engine as a submodule, and opens it. Everything is left staged, so the first commit is yours:

  ```
  .gitignore         .vscode/           assets/icon.png    sce_sys/
  CMakeLists.txt     build.hxml         assets/white.png   src/Main.hx
  3ds/icon.png                          project.fried      switch/icon.jpg
  ```

  `.vscode/` is the engine repository's configuration retargeted at the game. The art in `assets/`, `sce_sys/`, `switch/` and `3ds/` is flat white placeholder to replace.
- **Open Project** and **Save** read and write `project.fried`, keeping any keys the tool does not know about. **Close Project** (Ctrl+W) goes back to the home screen. Anything that would discard unsaved edits offers to save them first.
- **Build** (Ctrl+B) runs the whole pipeline for the selected platform in a window that streams the output, colours it by message type, reports when it is done and offers to open the build folder:

  ```sh
  haxe build.hxml
  cmake -S . -B build[/vita|/switch|/3ds] -DCMAKE_BUILD_TYPE=Debug [-DCMAKE_TOOLCHAIN_FILE=<the console's>]
  cmake --build build[/vita|/switch|/3ds] --config Debug --parallel
  ```

  A Vita build needs `VITASDK` set and a Switch or 3DS build needs `DEVKITPRO`, and all of them need `haxe` and `cmake` on the `PATH` the tool itself was started with.
- **Set Icon...** writes a project's four icons from one source image, showing what each becomes first: `assets/icon.png` for the window, `sce_sys/icon0.png` for the Vita, `switch/icon.jpg` for the Switch and `3ds/icon.png` for the 3DS.
- **Display** sets how the game fills the screen: at the screen's native resolution, or scaled from a size it is designed for by fit, integer scale, expand or stretch, with a nearest or linear filter. **Different on** gives the selected platform a display of its own instead of the one every platform shares.
- **Build type** and **Build directory** are remembered per project and per platform, defaulting to `Debug` and to `build`, `build/vita`, `build/switch` and `build/3ds`. The directory field is editable in full, with `{project}` standing for the project's own directory.

A generated project also compiles by hand: `haxe build.hxml`, then `cmake -S . -B build && cmake --build build`, or the same with `-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake` for a `.vpk` `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake` for a `.nro` and `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake` for a `.3dsx`. Cloning the submodule needs `git` and network access to the engine repository.
