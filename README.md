# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterwards: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares. It builds one for PC, the PlayStation Vita, the Nintendo Switch, the Nintendo 3DS and the Casio fx-CG50, all from the same sources, and from Linux it also builds the PC game for Windows.

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
  cg50/icon-uns.png  cg50/icon-sel.png
  ```

  `.vscode/` is the engine repository's configuration retargeted at the game. It debugs with gdb on Linux and with Visual Studio's debugger on Windows, so F5 on Windows needs Visual Studio's C++ tools and a `cmake` on the `PATH`, not the w64devkit the tool can download; a `build/` the tool configured with w64devkit has to be deleted before VS Code builds into it. The art in `assets/`, `sce_sys/`, `switch/`, `3ds/` and `cg50/` is flat white placeholder to replace.
- **An open project** is a sidebar of pages: **General** for the game's identity, **Display**, **Icons**, **Engine**, and one page per platform with what only that platform declares. The build controls stay at the bottom whichever page is shown, and **Projects** above the sidebar goes back to the home screen.
- **Open Project** and **Save** read and write `project.fried`, keeping any keys the tool does not know about. The window title is marked while the form holds unsaved edits. **Close Project** (Ctrl+W) goes back to the home screen. Anything that would discard unsaved edits offers to save them first.
- **Build** (Ctrl+B) runs the whole pipeline for the selected target in a window that streams the output, colours it by message type, shows how long it has been running, reports how long it took once it is done and offers to open the build folder:

  ```sh
  haxe build.hxml
  cmake -S . -B build[/windows|/vita|/switch|/3ds|/cg50] -DCMAKE_BUILD_TYPE=Debug [-DCMAKE_TOOLCHAIN_FILE=<the target's>]
  cmake --build build[/windows|/vita|/switch|/3ds|/cg50] --config Debug --parallel
  ```

  A Windows build from Linux needs the MinGW-w64 cross compiler, `x86_64-w64-mingw32-g++`, on the `PATH` and uses the toolchain file the engine ships in `engine/cmake/toolchains/mingw-w64.cmake`; Wine runs the result. A Vita build needs `VITASDK` set, a Switch or 3DS build needs `DEVKITPRO`, and an fx-CG50 build needs the fxSDK's `fxsdk` command on the `PATH`, which is how its toolchain file is found. All of them need `haxe` on the `PATH` the tool itself was started with, and every one but a PC build on Windows needs `cmake` there too.

  On Windows, a PC build uses Visual Studio's C++ tools when they are installed, with the `cmake` on the `PATH` or else the one Visual Studio installs for itself. Without them, the first build offers to download [w64devkit](https://github.com/skeeto/w64devkit) 2.10.1, about 70 MB with GCC, CMake and Ninja, into `%LOCALAPPDATA%\Fried Engine\Fried Project Manager\tools`, checks it against its known SHA-256 and builds with it from then on, with the Ninja generator, so the executable lands directly in the build folder. Neither one is put on the system `PATH`. Switching between the two needs a fresh build folder, since CMake keeps the generator it first configured a folder with.
- **Change Icons...** on the Icons page writes a project's icons from one source image, showing what each becomes first: `assets/icon.png` for the window, `sce_sys/icon0.png` for the Vita, `switch/icon.jpg` for the Switch, `3ds/icon.png` for the 3DS, and `cg50/icon-uns.png` and `cg50/icon-sel.png` for the fx-CG50, the menu icon unselected and selected.
- **Storage...** on the home screen lists what builds use outside the projects, grouped by platform and with the size of each: for PC, the engine's dependency sources, unpacked once per machine for every PC build, and the w64devkit the tool installs on Windows; for the consoles and the calculator, the SDK each one builds with. What was downloaded can be deleted from there, which only means downloading it again the next time a build needs it. An SDK is only shown, since it is removed with the installer that put it there.
- **Change Version...** on the Engine page moves the project's `engine` submodule to another branch or tag of the engine repository, listed straight from it, with the one checked out marked as current. The project's own repository records the change once `engine` is committed.
- **Display** sets how the game fills the screen: at the screen's native resolution, or scaled from a size it is designed for by fit, integer scale, expand or stretch, with a nearest or linear filter. A platform's own page can give it a display of its own instead of the one every platform shares. A new project starts with every platform on a display of its own at native resolution, since each console's screen differs.
- **LiveArea** on the Vita page sets the background (840x500) and start button (280x158) of the game's page on the Vita's home screen, converted to the palette PNGs the console reads, and its layout: the start button in the centre, or on the right with a title, subtitle and footer beside it. All of it is written on Save; a `template.xml` edited by hand is left alone unless replaced on purpose.
- **Boot Screen** on the Vita page sets `sce_sys/pic0.png`, the 960x544 image the console shows while the game starts. It is optional: **Remove** drops it on Save, and the engine packs it only when it is there.
- **Build type** and **Build directory** are remembered per project and per target, defaulting to `Debug` and to `build`, `build/windows`, `build/vita`, `build/switch`, `build/3ds` and `build/cg50`. The directory field is editable in full, with `{project}` standing for the project's own directory.

A generated project also compiles by hand: `haxe build.hxml`, then `cmake -S . -B build && cmake --build build`, or the same with `-DCMAKE_TOOLCHAIN_FILE=engine/cmake/toolchains/mingw-w64.cmake` for a Windows `.exe` from Linux, `-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake` for a `.vpk`, `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake` for a `.nro`, `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/3DS.cmake` for a `.3dsx` and `-DCMAKE_TOOLCHAIN_FILE=<fxSDK prefix>/lib/cmake/fxsdk/FXCG50.cmake` for a `.g3a`. Cloning the submodule needs `git` and network access to the engine repository.
