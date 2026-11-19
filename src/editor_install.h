#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// Fried Editor releases from GitHub, one folder per version in the tool's data folder, so a new version installs beside the one in use.
namespace EditorInstall
{
    // The newest release this tool knows the checksum of.
    QString latestVersion();

    QString versionsDirectory();
    QString directory(const QString &version);
    QString executable(const QString &version);
    bool isInstalled(const QString &version);

    // Complete installs only, newest first.
    QStringList installedVersions();

    // Where the download is unpacked, and where the install commands have to run.
    QString unpackDirectory(const QString &version);

    // Clears what an earlier, interrupted install of this version left, then lists the commands that download, check and unpack it; finishInstall() puts it in place.
    QList<QStringList> beginInstall(const QString &version);
    bool finishInstall(const QString &version, bool unpacked, QString *error);
}
