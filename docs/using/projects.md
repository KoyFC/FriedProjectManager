# Projects

**Home screen** with the last 20 projects opened, most recent first, each with its icon, name and directory. One whose `project.fried` is gone is listed as **(not found)**, and **Locate...** points it at where it moved to. For one that is found, the same button reads **Open Folder** and shows its folder in the file manager. **Remove** takes an entry off the list without deleting anything.

**New Project** writes a whole project directory, makes it a Git repository with the engine as a submodule, and opens it. Everything is left staged, so the first commit is yours:

```
.gitignore         .vscode/           assets/icon.png    sce_sys/
CMakeLists.txt     build.hxml         assets/white.png   src/nodes/
3ds/icon.png       editor.hxml        project.fried      switch/icon.jpg
cg50/icon-uns.png  cg50/icon-sel.png  nds/icon.bmp       assets/scenes/
```

The game has no `main` of its own: `build.hxml` names the engine's `fried.Entry`, which opens the window and loads the scene `project.fried` names, so what the game is made of is its nodes in `src/nodes/` and its scenes, made in [Fried Editor](https://github.com/KoyFC/FriedEditor), and updating the engine updates how a game starts. `.vscode/` marks `engine/` read-only, so the submodule is not edited by accident and an update still replaces it. `.vscode/` is the engine repository's configuration retargeted at the game. It debugs with gdb on Linux and with Visual Studio's debugger on Windows, so F5 on Windows needs Visual Studio's C++ tools and a `cmake` on the `PATH`, not the w64devkit the tool can download; a `build/` the tool configured with w64devkit has to be deleted before VS Code builds into it. The art in `assets/`, `sce_sys/`, `switch/`, `3ds/`, `nds/` and `cg50/` is flat white placeholder to replace.

**An open project** is a sidebar of pages: **General** for the game's identity and the platforms it builds for (every one until some are unticked, written as `platforms`: only the ticked ones have a page and a build target, the engine refuses to configure a build for the others, and it checks the limits of the ticked ones on every build), **Display** (which also holds vsync, the frame rate the game starts at and the physics rate, the fixed steps a second its `fixedUpdate` and kinematic bodies run at, written as `physicsRate`), **Icons**, **Engine**, and one page per platform with what only that platform declares, which includes its **Precision**: whether Haxe's `Float` is a 32-bit float (single) or a 64-bit double in that platform's build, written as `precision` in the platform's section and only when it is not the default. The DS defaults to single, every other platform to double, and the calculator offers no choice. The build controls stay at the bottom whichever page is shown, and **Projects** above the sidebar goes back to the home screen.

**Open Project** and **Save** read and write `project.fried`, keeping any keys the tool does not know about. A project's folder given on the command line opens it at once. The window title is marked while the form holds unsaved edits. **Close Project** (Ctrl+W) goes back to the home screen. Anything that would discard unsaved edits offers to save them first.
