#pragma once

#include <QList>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

// The C++ compiler a PC build uses on Windows when Visual Studio is not installed: a w64devkit the tool downloads for itself.
namespace WindowsToolchain
{
    QString w64devkitDirectory();
    bool w64devkitInstalled();

    // Clears what an earlier, interrupted install left, then lists the commands
    // that download, check and unpack w64devkit; finishInstall() puts it in place.
    QList<QStringList> beginInstall();
    bool finishInstall(bool unpacked, QString *error);

    // For whatever the build runs that looks its tools up on the PATH.
    QProcessEnvironment w64devkitEnvironment();
}
