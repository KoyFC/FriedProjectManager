# Changelog

All notable changes to Fried Project Manager are written here, newest version first.

- Changes go under the topmost `## vX.Y.Z` heading while they are in development. That heading is the version in progress and has no date.
- When a version is published, replace `vX.Y.Z` with its number and add the date (`## v0.3.0 - 2026-12-01`), and start a new `## vX.Y.Z` section above it.
- Each section uses `Added`, `Changed`, `Fixed` and `Removed`, and leaves out the ones it has no entries for.
- The Release workflow copies a version's section into its GitHub release, see [Releasing](https://koyfc.github.io/FriedProjectManager/development/releasing/).

## vX.Y.Z

### Added
- Android page: download the Android tools and install the build on a connected device.
- Declare the platforms a project builds for.
- Choose a platform's float precision on its page.

### Changed
- Find the editor's build in `FriedEditor/build/app`.
- Build generated projects with full dead code elimination.
- The template scene has a root node, and the template's Quit node uses the engine's update signature.

## v0.4.0 - 2026-11-20

### Added
- Download and install Fried Editor releases (any published version), list them in Storage, show the editor on the home page and open projects in it. A local editor build is used before a downloaded one.
- Web and NDS build support.
- Pixel and fixed height display modes, one display shared by every platform, vsync, default target framerate and physics rate.
- Per-platform page: build target selection, toolchain and build commands, icon and 3DS/DS package text editing, asset check (also on the Vita page).
- Debug and log to file flags, remember the last opened directory, open a project folder or one given on the command line.
- New projects are made out of nodes and start through the engine entry.
- Documentation site on GitHub Pages, published with GitHub Actions.

### Changed
- Platforms, build targets, `project.fried`, build commands, icons, asset checks and Visual Studio lookup now go through FriedCore.
- Command output is decoded as UTF-8 or the code page.
- The desktop platform is named after the system.
- Licensed under MIT.

## v0.3.0 - 2026-10-19

### Added
- Engine version selection.
- Windows as a build target, Casio fx-CG50 and 3DS platforms.
- Per-platform display editor, Vita Live Area and XML editing, Pic0 boot splash selection.
- Build timer, storage management from the home screen and console SDKs in Storage.

### Changed
- Updated the project template (Main, sample GameObject and scene).

## v0.2.0 - 2026-09-27

### Added
- Switch platform and icon support for multiple platforms.
- Command colors and a compiling indicator.

### Changed
- The architecture moved from the README to `ARCHITECTURE.md`.

## v0.1.1 - 2026-09-26

- First tagged version.
