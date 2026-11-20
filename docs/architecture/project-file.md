# project.fried

The tool reads and rewrites a file the engine also reads, through FriedCore's `Project`, which keeps the parsed document whole and writes back only the fields the tool owns. Unknown keys survive a save, which is what lets the engine add one without the tool learning about it first. A project with no `display` has the native mode, as the engine reads it, so the Display page shows that and keeps the size it suggests (640x360) greyed out until a mode that uses it is chosen.

Emptying a field removes its key rather than storing an empty string, and that cascades: clearing `vita.titleId` removes the `vita` object with it, since the engine reads an absent key as undeclared.

`display` is one shared object, which the engine reads for every platform, so there is no display of a platform's own: a game picks a design size and a mode, and each screen scales it its own way. A `<platform>.display` left by an older project is taken out when the project is saved, since the engine refuses it at configure time. The size is written only for a mode that scales, which is also why two native displays compare equal whatever size the form holds for them. The names `buildTargetKey()` gives the remembered build settings are `pc`, `vita`, `switch`, `3ds`, `nds` and `cg50`, plus `windows` for the Windows build from another system and `web` for the web build.

`version` is validated as `NN.NN` because that is the shape a Vita package requires. The Switch's NACP takes any string, so the stricter rule wins and one field serves both.
