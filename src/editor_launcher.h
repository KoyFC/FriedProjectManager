#pragma once

#include <QString>

class QWidget;

// Starts Fried Editor on a project.
class EditorLauncher
{
public:
    // Where the editor's executable is: what was chosen before, the FRIED_EDITOR variable, or the build next to this repository's folder. Empty when none exists.
    static QString findExecutable();

    // Asks for the executable when it cannot be found, and remembers the answer. Reports what went wrong and returns false.
    static bool launch(const QString &projectDirectory, QWidget *parent);
};
