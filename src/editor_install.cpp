#include "editor_install.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVersionNumber>

#include <algorithm>
#include <functional>

namespace
{
    const QString s_repository = QStringLiteral("https://github.com/KoyFC/FriedEditor");
    const QString s_tagsPrefix = QStringLiteral("refs/tags/v");
    const QString s_peeledSuffix = QStringLiteral("^{}");
    const QString s_checksums = QStringLiteral("SHA256SUMS");

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

    QString releaseUrl(const QString &version, const QString &file)
    {
        return QStringLiteral("%1/releases/download/v%2/%3").arg(s_repository, version, file);
    }

    // The folder the unpacked release is in before it is moved into place.
    QString payloadDirectory(const QString &version)
    {
#ifdef Q_OS_WIN
        return QDir(EditorInstall::unpackDirectory(version)).filePath(QStringLiteral("files"));
#else
        return QDir(EditorInstall::unpackDirectory(version)).filePath(QStringLiteral("squashfs-root"));
#endif
    }

    // Only a folder that is named by a version and nothing else is a version.
    QVersionNumber versionNamed(const QString &name)
    {
        qsizetype end = 0;
        const QVersionNumber version = QVersionNumber::fromString(name, &end);
        return end == name.size() ? version : QVersionNumber();
    }
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

QStringList EditorInstall::listRemoteVersionsCommand()
{
    return {QStringLiteral("git"), QStringLiteral("ls-remote"), QStringLiteral("--tags"), s_repository + QStringLiteral(".git")};
}

QStringList EditorInstall::parseRemoteVersions(const QString &listing)
{
    QList<QVersionNumber> versions;
    for (const QString &line : listing.split(QChar('\n'), Qt::SkipEmptyParts))
    {
        const QStringList fields = line.split(QChar('\t'));
        if (fields.size() != 2 || !fields[1].startsWith(s_tagsPrefix))
        {
            continue;
        }

        // An annotated tag is listed a second time, peeled.
        QString name = fields[1].trimmed().mid(s_tagsPrefix.size());
        if (name.endsWith(s_peeledSuffix))
        {
            continue;
        }

        const QVersionNumber version = versionNamed(name);
        if (!version.isNull() && !versions.contains(version))
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
    QDir(unpackDirectory(version)).removeRecursively();
    QDir().mkpath(unpackDirectory(version));

    const QString asset = assetName(version);
    const QString download = QDir::toNativeSeparators(QDir(unpackDirectory(version)).filePath(asset));
    const QString checksums = QDir::toNativeSeparators(QDir(unpackDirectory(version)).filePath(s_checksums));

    // The checksum comes from the same release as the file, so it catches a download that was cut short or garbled.
    const QStringList fetchChecksums = {
#ifdef Q_OS_WIN
        QStringLiteral("curl.exe"),
#else
        QStringLiteral("curl"),
#endif
        QStringLiteral("--fail"), QStringLiteral("--location"), QStringLiteral("--retry"), QStringLiteral("3"),
        QStringLiteral("--output"), checksums, releaseUrl(version, s_checksums)};
    QStringList fetchAsset = fetchChecksums;
    fetchAsset[fetchAsset.size() - 2] = download;
    fetchAsset[fetchAsset.size() - 1] = releaseUrl(version, asset);

#ifdef Q_OS_WIN
    const QString hashCheck =
        QStringLiteral("$line = Get-Content -LiteralPath '%1' | Where-Object { $_ -like '*%2' }; "
                       "$actual = (Get-FileHash -Algorithm SHA256 -LiteralPath '%3').Hash; "
                       "if (-not $line -or ($line -split ' ')[0].ToUpper() -ne $actual) "
                       "{ Write-Error 'The download is not the Fried Editor %4 release.'; exit 1 }")
            .arg(checksums, asset, download, version);

    return {
        fetchChecksums,
        fetchAsset,
        {QStringLiteral("powershell.exe"), QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"), hashCheck},
        {QStringLiteral("powershell.exe"), QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"),
         QStringLiteral("Expand-Archive -LiteralPath '%1' -DestinationPath '%2' -Force")
             .arg(download, QDir::toNativeSeparators(payloadDirectory(version)))},
    };
#else
    return {
        fetchChecksums,
        fetchAsset,
        // Run in the unpacking folder, where the file the list names is.
        {QStringLiteral("sha256sum"), QStringLiteral("--check"), QStringLiteral("--ignore-missing"), s_checksums},
        {QStringLiteral("chmod"), QStringLiteral("+x"), download},
        // Leaves squashfs-root/ in the folder the command runs in.
        {download, QStringLiteral("--appimage-extract")},
    };
#endif
}

bool EditorInstall::finishInstall(const QString &version, bool unpacked, QString *error)
{
    if (unpacked)
    {
        QDir(directory(version)).removeRecursively();
        unpacked = QDir().rename(payloadDirectory(version), directory(version));
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
