#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// The Emscripten SDK a web build compiles with: the one a shell exported as
// EMSDK, one in ~/emsdk, or an emsdk the tool downloads for itself.
namespace EmscriptenSdk
{
    // Where the tool's own copy goes.
    QString managedDirectory();

    // Empty when there is none. Each candidate has to hold the toolchain file, so a half-finished install is skipped.
    QString directory();
    QString toolchainFile(const QString &directory);

    // Clears what an earlier, interrupted install left, then lists the commands
    // that clone emsdk and install and activate the Emscripten release this
    // tool was tested with; finishInstall() checks the result.
    QList<QStringList> beginInstall();
    bool finishInstall(bool installed, QString *error);
}
