# Project layout as the contract

The tool knows nothing about the engine beyond the layout `fried_add_game()` expects, and the engine knows nothing about the tool. That layout is the whole interface between them, which is why a generated project is ordinary enough to have been written by hand.

The templates are files under `templates/project/`, compiled into the binary by `qt_add_resources` and copied out with `@NAME@`-style placeholders substituted. Keeping them as files rather than as strings in the source means the template of a `CMakeLists.txt` is a `CMakeLists.txt`, editable and diffable as one. The list in `CMakeLists.txt` is explicit rather than a glob, so adding one is a deliberate act.

A new project is left staged but not committed, because the first commit of someone's game is theirs to write.
