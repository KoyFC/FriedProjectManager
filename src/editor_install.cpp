#include "editor_install.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QVersionNumber>

#include <algorithm>
#include <functional>

namespace
{
    const QString s_version = QStringLiteral("0.1.0");
    const QString s_releases = QStringLiteral("https://github.com/KoyFC/FriedEditor/releases/download");
    const QString s_linuxSha256 = QStringLiteral("ceb64e06129ebd6552d2d819210722553a077b95c0edf4d183a73e1a3491bdfa");
    const QString s_windowsSha256 = QStringLiteral("4474caf505f88cbcb1a037fef6d69739cc8fb711f5c50ef98998fa060117cb9d");

    // What an unfinished install is called, beside the folder it will become.
    const QString s_unpackingSuffix = QStringLiteral("-unpacking");

    QString assetName(const QString &version)
    {
#ifdef Q_OS_WIN
        return QStringLiteral("FriedEditor-%1-windows-x64.zip").arg(version);
#else
        return QStringLiteral("FriedEditor-%1-x86_64.AppImage").arg(version);
#endif
    }

    QString downloadPath(const QString &version)
    {
        return QDir(EditorInstall::versionsDirectory()).filePath(assetName(version));
    }

    QString checksumPath(const QString &version)
    {
        return downloadPath(version) + QStringLiteral(".sha256");
    }

    QString downloadUrl(const QString &version)
    {
        return QStringLiteral("%1/v%2/%3").arg(s_releases, version, assetName(version));
    }

    // Only a folder that is named by a version and nothing else is a version.
    QVersionNumber versionNamed(const QString &name)
    {
        qsizetype end = 0;
        const QVersionNumber version = QVersionNumber::fromString(name, &end);
        return end == name.size() ? version : QVersionNumber();
    }
}

QString EditorInstall::latestVersion()
{
    return s_version;
}

QString EditorInstall::versionsDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath(QStringLiteral("editor"));
}

QString EditorInstall::directory(const QString &version)
{
    return QDir(versionsDirectory()).filePath(version);
}

QString EditorInstall::unpackDirectory(const QString &version)
{
    return directory(version) + s_unpackingSuffix;
}

QString EditorInstall::executable(const QString &version)
{
    // The AppImage is unpacked at install time and its AppRun started directly, so no FUSE is needed to run it.
#ifdef Q_OS_WIN
    return QDir(directory(version)).filePath(QStringLiteral("fried_editor.exe"));
#else
    return QDir(directory(version)).filePath(QStringLiteral("AppRun"));
#endif
}

bool EditorInstall::isInstalled(const QString &version)
{
    const QFileInfo info(executable(version));
    return info.isFile() && info.isExecutable();
}

QStringList EditorInstall::installedVersions()
{
    QList<QVersionNumber> versions;
    for (const QFileInfo &folder : QDir(versionsDirectory()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        const QVersionNumber version = versionNamed(folder.fileName());
        if (!version.isNull() && isInstalled(folder.fileName()))
        {
            versions << version;
        }
    }
    std::sort(versions.begin(), versions.end(), std::greater<QVersionNumber>());

    QStringList names;
    for (const QVersionNumber &version : versions)
    {
        names << version.toString();
    }
    return names;
}

QList<QStringList> EditorInstall::beginInstall(const QString &version)
{
    QDir().mkpath(versionsDirectory());
    QFile::remove(downloadPath(version));
    QFile::remove(checksumPath(version));
    QDir(unpackDirectory(version)).removeRecursively();
    QDir().mkpath(unpackDirectory(version));

    const QString download = QDir::toNativeSeparators(downloadPath(version));
    const QString unpacked = QDir::toNativeSeparators(unpackDirectory(version));

#ifdef Q_OS_WIN
    const QString hashCheck =
        QStringLiteral("if ((Get-FileHash -Algorithm SHA256 -LiteralPath '%1').Hash -ne '%2') "
                       "{ Write-Error 'The download is not the Fried Editor %3 release.'; exit 1 }")
            .arg(download, s_windowsSha256.toUpper(), version);

    return {
        {QStringLiteral("curl.exe"), QStringLiteral("--fail"), QStringLiteral("--location"), QStringLiteral("--retry"),
         QStringLiteral("3"), QStringLiteral("--output"), download, downloadUrl(version)},
        {QStringLiteral("powershell.exe"), QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"), hashCheck},
        {QStringLiteral("powershell.exe"), QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"),
         QStringLiteral("Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force").arg(download, unpacked)},
    };
#else
    // sha256sum reads what to check from a file, which names the download by its full path.
    QFile checksum(checksumPath(version));
    if (checksum.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream(&checksum) << s_linuxSha256 << "  " << download << "\n";
    }

    return {
        {QStringLiteral("curl"), QStringLiteral("--fail"), QStringLiteral("--location"), QStringLiteral("--retry"),
         QStringLiteral("3"), QStringLiteral("--output"), download, downloadUrl(version)},
        {QStringLiteral("sha256sum"), QStringLiteral("--check"), QStringLiteral("--strict"), checksumPath(version)},
        {QStringLiteral("chmod"), QStringLiteral("+x"), download},
        // Leaves squashfs-root/ in the folder the command runs in, which is the unpacking one.
        {download, QStringLiteral("--appimage-extract")},
    };
#endif
}

bool EditorInstall::finishInstall(const QString &version, bool unpacked, QString *error)
{
    QFile::remove(downloadPath(version));
    QFile::remove(checksumPath(version));

    if (unpacked)
    {
        QDir(directory(version)).removeRecursively();
#ifdef Q_OS_WIN
        const QString root = unpackDirectory(version);
#else
        const QString root = QDir(unpackDirectory(version)).filePath(QStringLiteral("squashfs-root"));
#endif
        unpacked = QDir().rename(root, directory(version));
    }
    QDir(unpackDirectory(version)).removeRecursively();

    if (!unpacked || !isInstalled(version))
    {
        QDir(directory(version)).removeRecursively();
        *error = QStringLiteral("Fried Editor %1 could not be installed in %2.")
                     .arg(version, QDir::toNativeSeparators(directory(version)));
        return false;
    }

    return true;
}
