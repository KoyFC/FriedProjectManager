#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// Fried Editor releases from GitHub, one folder per version in the tool's data folder, so any published version installs beside the others.
namespace EditorInstall
{
    QString versionsDirectory();
    QString directory(const QString &version);
    QString executable(const QString &version);
    bool isInstalled(const QString &version);

    // Complete installs only, newest first.
    QStringList installedVersions();

    // The repository's tags, which is how releases are named, and the versions they are, newest first.
    QStringList listRemoteVersionsCommand();
    QStringList parseRemoteVersions(const QString &listing);

    // Where the download is unpacked, and where the install commands have to run.
    QString unpackDirectory(const QString &version);

    // Clears what an earlier, interrupted install of this version left, then lists the commands that download, check and unpack it; finishInstall() puts it in place.
    QList<QStringList> beginInstall(const QString &version);
    bool finishInstall(const QString &version, bool unpacked, QString *error);
}
