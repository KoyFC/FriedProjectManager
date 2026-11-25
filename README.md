# Fried Project Manager

A desktop application that creates [Fried Engine](https://github.com/KoyFC/FriedEngine) game projects and keeps editing them afterward: it generates a project that builds and runs out of the box, and reopens an existing one to change what its `project.fried` declares. It builds one for PC, the PlayStation Vita, the Nintendo Switch, the Nintendo 3DS, the Nintendo DS, the Casio fx-CG50, Android and the web, all from the same sources, and from Linux it also builds the PC game for Windows.

The engine lives in its own repository and is consumed by each game as a pinned Git submodule. This repository holds only the tool. See [Architecture](https://koyfc.github.io/FriedProjectManager/architecture/) for the decisions behind it.

Written in C++ with [Qt 6](https://www.qt.io/) and [CMake](https://cmake.org/), on top of [FriedCore](https://github.com/KoyFC/FriedCore).

## Documentation

The documentation is at [koyfc.github.io/FriedProjectManager](https://koyfc.github.io/FriedProjectManager/).

- [Installing](https://koyfc.github.io/FriedProjectManager/installing/)
- [Using the tool](https://koyfc.github.io/FriedProjectManager/using/projects/)
- [Architecture](https://koyfc.github.io/FriedProjectManager/architecture/)

## Building

[FriedCore](https://github.com/KoyFC/FriedCore), the library the tool shares with Fried Editor, is a submodule in `third_party/fried_core`, so clone with `--recurse-submodules`, or run `git submodule update --init` in a clone made without it.

```sh
cmake -S . -B build
cmake --build build
./build/fried_project_manager
```

## License

Fried Project Manager is released under the MIT License, see [`LICENSE`](LICENSE). Third party components are listed in [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
