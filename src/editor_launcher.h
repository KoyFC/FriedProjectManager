#pragma once

#include <QString>

class QWidget;

// Starts Fried Editor on a project: the one FRIED_EDITOR names, for working on the editor itself, or the newest the tool installed.
class EditorLauncher
{
public:
    // Empty when FRIED_EDITOR is not set to an executable.
    static QString overrideExecutable();

    // Empty when there is neither.
    static QString findExecutable();

    // Asks before downloading the newest release, and reports what went wrong. True once it is installed.
    static bool install(QWidget *parent);

    // Offers to download the editor when there is none. Reports what went wrong and returns false.
    static bool launch(const QString &projectDirectory, QWidget *parent);
};
