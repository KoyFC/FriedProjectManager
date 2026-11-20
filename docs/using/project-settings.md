# Project settings

**Display** sets how the game fills the screen: at the screen's native resolution, or scaled from a size it is designed for by fit, integer scale, expand, pixel or stretch, with a nearest or linear filter. Every platform uses the same display, and the pixel mode scales it by whole numbers on each screen, showing less of the world on a small one rather than a shrunken one. A new project starts in expand.

**Change Icons...** on the Icons page writes a project's icons from one source image, showing what each becomes first: `assets/icon.png` for the window, `sce_sys/icon0.png` for the Vita, `switch/icon.jpg` for the Switch, `3ds/icon.png` for the 3DS, `nds/icon.bmp` for the DS, and `cg50/icon-uns.png` and `cg50/icon-sel.png` for the fx-CG50, the menu icon unselected and selected.

Each platform's page also shows its own icon with a **Change Icon...** button, which writes only that platform's file from an image, at the size and in the format it reads.

The 3DS page has a **Description** and the DS page a **Second line** and a **Third line**, the package text those consoles show beside the name. Left empty they fall back to the version and to the organization and the version, and each stops accepting characters at what the package holds. They are written as `3ds.description`, `nds.subtitle1` and `nds.subtitle2`, which needs an engine that reads them.

The Vita, 3DS, DS and fx-CG50 pages check the project's `assets/` and package files against what that console takes and list what does not fit, from the same limits as the Fried Editor's Issues tab: an image in `sce_sys/` that is not a palette PNG, which the Vita's installer refuses, a texture over 1024 pixels a side on the 3DS and DS, a DS sound that is not mono PCM WAV, any sound on the fx-CG50, and more than 2048 KB of assets there. It only reports, since the editor is where a Vita image, an oversized texture or a stereo sound is fixed, with a copy of the original kept.

**LiveArea** on the Vita page sets the background (840x500) and start button (280x158) of the game's page on the Vita's home screen, converted to the palette PNGs the console reads, and its layout: the start button in the center, or on the right with a title, subtitle and footer beside it. All of it is written on Save; a `template.xml` edited by hand is left alone unless replaced on purpose.

**Boot Screen** on the Vita page sets `sce_sys/pic0.png`, the 960x544 image the console shows while the game starts. It is optional: **Remove** drops it on Save, and the engine packs it only when it is there.

**Change Version...** on the Engine page moves the project's `engine` submodule to another branch or tag of the engine repository, listed straight from it, with the one checked out marked as current. The project's own repository records the change once `engine` is committed.
