# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterwards: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares. It builds one for PC, the PlayStation Vita and the Nintendo Switch, all from the same sources.

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
                                        project.fried      switch/icon.jpg
  ```

  `.vscode/` is the engine repository's configuration retargeted at the game: build, run and debug tasks, the `host-*.sh` wrappers that forward commands to the host when the editor is sandboxed, and the C++ include paths. `sce_sys/` holds the four files a `.vpk` needs and `switch/icon.jpg` is the one a `.nro` carries, all with flat white placeholder art to replace.
- The generated `src/Main.hx` calls `window.setIcon(Assets.game("icon.png"))`, so every icon a project has is live from the first build: `assets/icon.png` is the window icon the game loads at runtime, `sce_sys/icon0.png` is what the Vita's installer shows, and `switch/icon.jpg` is what `elf2nro` packs into the `.nro`. They cannot be one file, because the Vita only accepts a 128x128 palette PNG and the Switch a 256x256 JPEG, while a window icon is truecolor of any size. All three ship flat white.
- Creating one first shows a notice that it runs `git` and clones the engine repository, with a **Don't show this again** toggle. That choice is stored as `newProject/showGitNotice` in the tool's settings (`~/.config/Fried Engine/Fried Project Manager.conf` on Linux), so deleting the key brings the notice back.
- **Open Project** reads an existing `project.fried`, and **Save** writes the edited fields back, keeping any keys the tool does not know about.
- The platform selector hides the fields the chosen platform does not use, and **Build** (Ctrl+B, or the button beside it) runs the whole pipeline for it in a window that streams the output:

  ```sh
  haxe build.hxml
  cmake -S . -B build[/vita|/switch] -DCMAKE_BUILD_TYPE=Debug [-DCMAKE_TOOLCHAIN_FILE=<the console's>]
  cmake --build build[/vita|/switch] --config Debug --parallel
  ```

  The toolchain file is `$VITASDK/share/vita.toolchain.cmake` for the Vita and `$DEVKITPRO/cmake/Switch.cmake` for the Switch. Nothing else about the pipeline changes with the platform, because which platform a project is being built for is something the toolchain file tells the engine's CMake rather than something the tool passes along.

  The window that streams the output carries an **Open Build Folder** button, so the artifacts are one click away when it finishes. It stays disabled while there is no such directory to open, which is what a build that failed before creating one leaves behind.

  That output is coloured by what each line is: the command being run, an error, a warning, and make's percentages and CMake's status lines receding into the background. None of it comes from the tools, which turn their own colour off when they are writing to a pipe rather than to a terminal; each line is recognised by what it looks like, so the same rules cover `haxe`, `cmake` and the compiler. Every colour is derived from the current theme, since a red dark enough to read on white is too dark to read on near-black.

  Under the log a line reports that the build is still going, as `Compiling` with the dots cycling from none to three, and ends as `Done.` or `Failed.` in the same colours the log uses. It reserves the width of the longest thing it can say, so the word does not move while the dots come and go.

  A Vita build needs `VITASDK` set and a Switch build needs `DEVKITPRO`, and all three need `haxe` and `cmake` on the `PATH` the tool itself was started with. Both SDKs export their variable from a shell profile, so a tool launched from a desktop menu may never have inherited it, and that is reported as a missing toolchain rather than as a build that fails halfway. If the form holds edits the file does not, Build offers to save them first, since the build reads `project.fried` and not the form.
- **Set Icon...** opens a dialog with a slot per platform, each previewing what its file would become before anything is written: `assets/icon.png` truecolor for the window, `sce_sys/icon0.png` as the 128x128 palette PNG the Vita's installer demands, and `switch/icon.jpg` as the 256x256 JPEG a `.nro` carries. The console icons follow the window icon by default, and unticking **Same image as the window icon** lets you give each one a different image, which is what a console icon usually wants. Each slot says what it had to do to the source: an oblong image keeps its centre square, anything over 512x512 is reduced for the window, a source under the console's size is enlarged and warns it will look soft, more than 256 colours says so because that is all a palette holds, and a source with transparency says the Switch icon was flattened onto black, because a JPEG holds none. A slot you leave alone is not rewritten, so one icon can be changed without touching the others. Every file is staged before any of them is replaced, so a failure leaves all three as they were, and the copy next to the executable is refreshed by the next build.
- **Build type** picks `Debug` or `Release`, and is remembered per project and per platform, so a project can keep symbols on PC while the Vita build is optimised. Debug is the default and keeps what a debugger needs; Release optimises, which is what a build for players wants. It reaches CMake twice, as `-DCMAKE_BUILD_TYPE` when configuring and as `--config` when building, because a generator that holds every configuration at once ignores the first and reads the second. Before this the tool passed neither, so a game was compiled with no optimisation and no symbols.
- **Build directory** is a field of its own, remembered per project and per platform, defaulting to `build`, `build/vita` and `build/switch`. The field holds the whole path, all of it editable, with `{project}` standing for the project's own directory: `{project}/build` is inside the project and travels with it, while anything absolute is shown and used as it is. A path typed without the token is read as relative to the project, and the tooltip always shows what the field resolves to. **Reset** puts back the default for the platform currently selected, and is disabled while the field already holds it. For a Vita build only, the field warns when a space in the project's path or in the build directory would let the build compile but not be packaged. Only the CMake build tree moves: `build.hxml` writes the generated C++ to `build/cpp` and `fried_add_game()` reads it from there, so that part is the engine's to decide.

A generated project compiles and runs as it comes out: `haxe build.hxml`, then `cmake -S . -B build && cmake --build build`, or the same with `-DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake` for a `.vpk` and with `-DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake` for a `.nro`. Cloning the submodule needs `git` and network access to the engine repository.

VitaSDK's packaging step does not quote the paths it is handed, so a space anywhere in a project's path lets it compile for the Vita but not be packaged. The New Project dialog handles that: the project keeps the name as typed, while its directory replaces the spaces with dashes (`My Demo Game` goes in `My-Demo-Game`), and it says so. If the location you picked contains a space of its own, which the tool cannot rename, it warns instead.
