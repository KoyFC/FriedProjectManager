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

- With no project open the window shows a **home screen** listing the projects opened before, most recent first, each with its `assets/icon.png`, the name its `project.fried` declares, and the directory it lives in. The icon is read from disk every time the list is shown, so changing it shows up straight away, and a project without one keeps an empty box of the same width so the rows stay aligned. Opening one from the list, or creating or opening one any other way, moves it to the top. The list is kept as `recentProjects/paths` in the tool's settings and holds the last 20.
- An entry whose `project.fried` is gone is listed as **(not found)** and cannot be opened. **Locate...** points it at where the project moved to, keeping it on the list, and opening one by double-click offers the same thing rather than reporting an error.
- **Remove** takes any entry off the list, found or not. It deletes nothing: it says so before doing it, names the directory the project stays in, and opening that project again puts it back.
- **Close Project** (Ctrl+W) goes back to that list, and offers to save first if the form holds edits the file does not.
- **New Project** writes a whole project directory, makes it a Git repository with the engine as a submodule, and opens it. Everything is staged, so the first commit is yours:

  ```
  .gitignore         .vscode/           assets/icon.png    sce_sys/
  CMakeLists.txt     build.hxml         assets/white.png   src/Main.hx
                                        project.fried
  ```

  `.vscode/` is the engine repository's configuration retargeted at the game: build, run and debug tasks, the `host-*.sh` wrappers that forward commands to the host when the editor is sandboxed, and the C++ include paths. `sce_sys/` holds the four files a `.vpk` needs, with flat white placeholder art to replace.
- The generated `src/Main.hx` calls `window.setIcon(Assets.game("icon.png"))`, so both icons a project has are live from the first build: `assets/icon.png` is the window icon on PC, and `sce_sys/icon0.png` is what the Vita's installer shows. They cannot be one file, because the console only accepts a 128x128 palette PNG while a window icon is truecolor. Both ship flat white.
- Creating one first shows a notice that it runs `git` and clones the engine repository, with a **Don't show this again** toggle. That choice is stored as `newProject/showGitNotice` in the tool's settings (`~/.config/Fried Engine/Fried Project Manager.conf` on Linux), so deleting the key brings the notice back.
- **Open Project** reads an existing `project.fried`, and **Save** writes the edited fields back, keeping any keys the tool does not know about.
- The platform selector hides the fields the chosen platform does not use, and **Build** (Ctrl+B, or the button beside it) runs the whole pipeline for it in a window that streams the output:

  ```sh
  haxe build.hxml
  cmake -S . -B build[/vita] [-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake]
  cmake --build build[/vita] --parallel
  ```

  A Vita build needs `VITASDK` set, and both need `haxe` and `cmake` on the `PATH` the tool itself was started with. If the form holds edits the file does not, Build offers to save them first, since the build reads `project.fried` and not the form.
- **Set Icon...** opens a dialog with a slot per platform, each previewing what its file would become before anything is written: `assets/icon.png` truecolor for the window, and `sce_sys/icon0.png` as the 128x128 palette PNG the console's installer demands. The Vita icon follows the window icon by default, and unticking **Same image as the window icon** lets you give it a different image, which is what a console icon usually wants. Each slot says what it had to do to the source: an oblong image keeps its centre square, anything over 512x512 is reduced for the window, a source under 128x128 is enlarged for the Vita and warns it will look soft, and more than 256 colours says so, since that is all a palette holds. A slot you leave alone is not rewritten, so the Vita icon can be changed without touching the window icon. Both files are staged before either is replaced, so a failure leaves the pair as it was, and the copy next to the executable is refreshed by the next build.
- **Build directory** is a field of its own, remembered per project and per platform, defaulting to `build` and `build/vita`. A path inside the project is kept relative, and an absolute path builds wherever you point it. Only the CMake build tree moves: `build.hxml` writes the generated C++ to `build/cpp` and `fried_add_game()` reads it from there, so that part is the engine's to decide.

A generated project compiles and runs as it comes out: `haxe build.hxml`, then `cmake -S . -B build && cmake --build build`, or the same with `-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake` for a `.vpk`. Cloning the submodule needs `git` and network access to the engine repository.

VitaSDK's packaging step does not quote the paths it is handed, so a space anywhere in a project's path lets it compile for the Vita but not be packaged. The New Project dialog handles that: the project keeps the name as typed, while its directory replaces the spaces with dashes (`My Demo Game` goes in `My-Demo-Game`), and it says so. If the location you picked contains a space of its own, which the tool cannot rename, it warns instead.
