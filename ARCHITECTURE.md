# Fried Project Manager Architecture

This document describes the decisions behind what is currently implemented, and why each one was made. See `README.md` for what the tool does, and the engine's own `ARCHITECTURE.md` for the layout every project here is written to.

## Project layout as the contract

The tool knows nothing about the engine beyond the layout `fried_add_game()` expects, and the engine knows nothing about the tool. That layout is the whole interface between them, which is why a generated project is ordinary enough to have been written by hand.

The templates are files under `templates/project/`, compiled into the binary by `qt_add_resources` and copied out with `@NAME@`-style placeholders substituted. Keeping them as files rather than as strings in the source means the template of a `CMakeLists.txt` is a `CMakeLists.txt`, editable and diffable as one. The list in `CMakeLists.txt` is explicit rather than a glob, so adding one is a deliberate act.

A new project is left staged but not committed, because the first commit of someone's game is theirs to write.

## project.fried

The tool reads and rewrites a file the engine also reads, so it keeps the parsed document whole and writes back only the fields it owns. Unknown keys survive a save, which is what lets the engine add one without the tool learning about it first.

Emptying a field removes its key rather than storing an empty string, and that cascades: clearing `vita.titleId` removes the `vita` object with it, since the engine reads an absent key as undeclared.

`display` is edited as the engine reads it: one shared object, and an override under each platform's own section, `pc`, `vita`, `switch` or `3ds`, which are the same names `platformKey()` gives the remembered build settings. The engine merges an override key by key, but the form shows a platform as either shared or wholly its own, so an override is written with every key and starts as a copy of the shared display. A display the form did not change is not rewritten, so one written by hand with fewer keys keeps its shape. The size is written only for a mode that scales, which is also why two native displays compare equal whatever size the form holds for them.

`version` is validated as `NN.NN` because that is the shape a Vita package requires. The Switch's NACP takes any string, so the stricter rule wins and one field serves both.

## Platforms

A platform is a `Platform` enum value, a build directory, a toolchain file and nothing else. The tool never tells the engine's CMake which platform it is building for: the toolchain file does that by setting `VITA`, `NINTENDO_SWITCH` or `NINTENDO_3DS`, so the pipeline is the same three commands everywhere and adding a platform here is a case in two functions rather than a new code path.

Each SDK is found through its own environment variable (`VITASDK`, `DEVKITPRO`) and checked for the toolchain file before anything runs, so a missing SDK is one dialog rather than a build that dies halfway. Both variables are normally exported from a shell profile, which a tool launched from a desktop menu may never have inherited.

The build directory and build type are remembered per project and per platform, so a project can keep symbols on PC while its console build is optimised. Only the CMake build tree moves with that field: `build.hxml` writes the generated C++ to `build/cpp` and `fried_add_game()` reads it from there, so where that lands is the engine's to decide and not the tool's. The build type reaches CMake twice, as `-DCMAKE_BUILD_TYPE` when configuring and as `--config` when building, because a generator that holds every configuration at once ignores the first and reads the second.

VitaSDK's packaging step does not quote the paths it is handed, so a space anywhere in a Vita project's path lets it compile but not be packaged. New Project avoids it by putting `My Demo Game` in `My-Demo-Game` while the project keeps the name as typed, and the build form warns when a path it cannot rename has one. `elf2nro` quotes its arguments, so the Switch needs none of this.

## Icons

A project keeps one icon per platform because no single file can serve them all: the window icon is truecolor of any size, the Vita's installer reads a 128x128 palette PNG and rejects a truecolor one, a `.nro` carries a 256x256 JPEG, and a `.3dsx` a 48x48 icon with no transparency. So `Icon` is addressed by platform throughout, and the dialog holds a panel per platform rather than named members.

Each conversion reports what it had to do to the source, because silently cropping or flattening someone's artwork is worse than saying so. A source with transparency is composited onto black for the Switch and the 3DS rather than converted, so the result does not depend on whatever colour sat under the transparent pixels.

Every file is staged before any of them is committed, so a failure on the third icon leaves the first two as they were. The cropping, resizing, palette reduction and staged writing live in `ImageConversion`, shared with the LiveArea images.

## LiveArea images

The Vita page edits `sce_sys/livearea/contents/bg.png` and `startup.png`, the two images the engine packs beside `icon0.png`. Unlike the icons they are part of the form: a chosen image waits and is written on Save, before `project.fried`, so Save, Discard and the unsaved marker treat it the way they treat a field. One the console would reject, at the wrong size or truecolor, is converted when the project opens and waits for Save the same way, rather than being reported and left broken.

`template.xml` offers only what homebrew is known to get right: `a1`, which centres the start button, and `psmobile`, which puts it on the right with three lines of text beside it laid out the way VitaShell lays out its own. Where each style's frames sit is not documented anywhere, so a fuller editor would be guesswork. A file counts as the form's when writing what the form read back from it gives the same document, compared without regard to indentation or attribute order. Anything else was edited by hand and is never rewritten silently, since the form would lose whatever it does not show.

## The command window

One dialog runs every sequence of commands the tool issues, streaming a merged stdout and stderr into a read-only log.

Its colours are decided by what each line looks like, not by ANSI codes. The tools turn their own colour off when they write to a pipe rather than to a terminal, so forcing it on would mean a switch per tool and a parser for the escape sequences; recognising `error:`, `CMake Warning`, make's percentages and haxe's diagnostics covers all three tools with one set of rules. The colours come from the current palette, since a red dark enough to read on white is too dark to read on near-black.

The log is written with a text cursor rather than as HTML, because HTML would have to be escaped and compiler output is full of `<` and `&`.

A read of the pipe ends wherever it ran dry rather than at a line break, so the tail of a chunk is held back until the rest of its line arrives. Without that, colouring per line would split one line into two of different colours.

The activity the dialog reports is set by whoever opens it, so the build window says `Compiling` while the one running `git` does not claim to be compiling.

## Settings

`QSettings` under `Fried Engine/Fried Project Manager`, which is `~/.config/Fried Engine/Fried Project Manager.conf` on Linux:

- `recentProjects/paths`: the home screen's list, most recent first, capped at 20.
- `newProject/showGitNotice`: whether the notice about running `git` is still shown.
- `buildDirectories/<platform>/<project>` and `buildTypes/<platform>/<project>`: per project and per platform, with the project path percent encoded so it stays one key.
