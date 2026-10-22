#pragma once

#include <QList>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

// The C++ compiler a PC build uses on Windows: Visual Studio when it is
// installed, and otherwise a w64devkit the tool downloads for itself.
namespace WindowsToolchain
{
    // Empty when there is no Visual Studio with its C++ tools, or no CMake to drive it.
    QString visualStudioCmake();

    QString w64devkitDirectory();
    bool w64devkitInstalled();

    // Clears what an earlier, interrupted install left, then lists the commands
    // that download, check and unpack w64devkit; finishInstall() puts it in place.
    QList<QStringList> beginInstall();
    bool finishInstall(bool unpacked, QString *error);

    // Absolute paths, since the tool's own PATH never has w64devkit on it.
    QList<QStringList> w64devkitPipeline(const QStringList &haxe, const QString &buildTree, const QString &buildType);

    // For whatever the build runs that looks its tools up on the PATH.
    QProcessEnvironment w64devkitEnvironment();
}
