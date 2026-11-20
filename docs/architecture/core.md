# FriedCore

What a project file means is also what Fried Editor needs to know, so it is written once, in [FriedCore](https://github.com/KoyFC/FriedCore), a library with no Qt in it that both tools take as a submodule. The tool's own sources are its windows and what only it does. Text crosses between the two through `core_bridge.h`, as FriedCore's is UTF-8 in a `std::string`.

So far the platforms, the build targets, the project file, the commands of a build, the icons and the check of a project's assets come from it.
