# Settings

`QSettings` under `Fried Engine/Fried Project Manager`, which is `~/.config/Fried Engine/Fried Project Manager.conf` on Linux:

- `recentProjects/paths`: the home screen's list, most recent first, capped at 20.
- `newProject/showGitNotice`: whether the notice about running `git` is still shown.
- `lastLocations/projects` and `lastLocations/images`: the folder the next file dialog of that kind opens in. Choosing a project folder remembers the folder that holds it, since the next one wanted is usually a sibling, and choosing an image remembers the folder the image is in. Both the Open Project and the New Project dialogs share the first, and the image editor and the icon dialog share the second. A folder that no longer exists is ignored for what the dialog would have opened in anyway.
- `buildDirectories/<target>/<project>`, `buildTypes/<target>/<project>` and `buildLogToFile/<target>/<project>`: per project and per target, with the project path percent encoded so it stays one key.
