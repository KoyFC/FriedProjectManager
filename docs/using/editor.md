# Fried Editor

**Open in Editor** (Ctrl+E, or the button at the top of the project sidebar) starts [Fried Editor](https://github.com/KoyFC/FriedEditor) on the project. The first time, the tool offers to download a Fried Editor release from GitHub into its own data folder, and from then on opens the newest one installed; For working on the editor itself, a local build is used instead of any download: the one `FRIED_EDITOR=<executable>` names, or the one in a `FriedEditor` checkout next to this repository's folder (`FriedEditor/build/app_imgui`). Before opening, it checks that the machine can build what the editor's player needs, the same as a PC build, and says what is missing, offering the compiler download on Windows.

The home screen says which Fried Editors are installed, and **Editor Versions...** lists every published release to download another beside them (git has to be installed to list them). The newest installed opens projects, and Storage deletes the others.
