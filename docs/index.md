# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterward: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares. It builds one for PC, the PlayStation Vita, the Nintendo Switch, the Nintendo 3DS, the Nintendo DS, the Casio fx-CG50 and the web, all from the same sources, and from Linux it also builds the PC game for Windows.

The engine lives in its own repository and is consumed by each game as a pinned Git submodule. This repository holds only the tool. See [Architecture](architecture/index.md) for the decisions behind it.

Written in C++ with [Qt 6](https://www.qt.io/) and [CMake](https://cmake.org/), on top of [FriedCore](https://github.com/KoyFC/FriedCore).

- [Installing](installing.md): the releases, and building the tool from source.
- [Using](using/projects.md): projects, their settings, building, the Editor and storage.
- [Architecture](architecture/index.md): the decisions behind what is implemented, and why.
- [License](https://github.com/KoyFC/FriedProjectManager/blob/main/LICENSE): MIT, with the third party notices next to it.
