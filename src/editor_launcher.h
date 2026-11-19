#pragma once

#include <QString>

class QWidget;

// Starts Fried Editor on a project: a local build, for working on the editor itself, or the newest the tool installed.
class EditorLauncher
{
public:
    // The editor being developed: what FRIED_EDITOR names, or the build in a FriedEditor checkout next to this repository's folder. Empty when there is neither.
    static QString localExecutable();

    // Empty when there is neither.
    static QString findExecutable();

    // Asks before downloading the newest release, and reports what went wrong. True once it is installed.
    static bool install(QWidget *parent);

    // Offers to download the editor when there is none. Reports what went wrong and returns false.
    static bool launch(const QString &projectDirectory, QWidget *parent);
};
