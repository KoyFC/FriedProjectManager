#include "storage.h"

#include "windows_toolchain.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>

namespace
{
    const QString s_unpackedMarker = QStringLiteral(".fried-unpacked");
    const QString s_installedMarker = QStringLiteral(".fried-installed");
    const QString s_installedDirectory = QStringLiteral("installed");
    const QString s_pc = QStringLiteral("PC");

    void appendSdk(QList<StorageItem> &items, const QString &name, const QString &platform, const QString &path)
    {
        if (!path.isEmpty() && QFileInfo(path).isDir())
        {
            items << StorageItem{name, platform, QDir::cleanPath(path), StorageItem::Origin::ItsOwnInstaller};
        }
    }

    // The fxSDK keeps its compiler and libraries in <prefix>/share/fxsdk, beside the bin/ holding the command.
    QString fxsdkDirectory()
    {
        const QString fxsdk = QStandardPaths::findExecutable(QStringLiteral("fxsdk"));
        if (fxsdk.isEmpty())
        {
            return QString();
        }

        return QFileInfo(fxsdk).absoluteDir().filePath(QStringLiteral("../share/fxsdk"));
    }
}

QString Storage::dependencySourcesDirectory()
{
#ifdef Q_OS_WIN
    const QString base = qEnvironmentVariable("LOCALAPPDATA");
#else
    const QString xdgCache = qEnvironmentVariable("XDG_CACHE_HOME");
    const QString base = xdgCache.isEmpty() ? QDir::home().filePath(QStringLiteral(".cache")) : xdgCache;
#endif
    return QDir(base).filePath(QStringLiteral("fried-engine/deps"));
}

QList<StorageItem> Storage::items()
{
    QList<StorageItem> found;

    const QDir sources(dependencySourcesDirectory());
    for (const QFileInfo &release : sources.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        if (release.fileName() == s_installedDirectory)
        {
            continue;
        }

        found << StorageItem{release.fileName(), s_pc, release.absoluteFilePath(), StorageItem::Origin::EngineBuilds,
                             QFileInfo::exists(QDir(release.absoluteFilePath()).filePath(s_unpackedMarker))};
    }

    // The libraries the engine compiled from those sources, one folder per compiler and configuration.
    const QDir installed(sources.filePath(s_installedDirectory));
    for (const QFileInfo &build : installed.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        found << StorageItem{build.fileName(), s_pc, build.absoluteFilePath(), StorageItem::Origin::EngineBuilds,
                             QFileInfo::exists(QDir(build.absoluteFilePath()).filePath(s_installedMarker))};
    }

    const QString w64devkit = WindowsToolchain::w64devkitDirectory();
    if (QFileInfo::exists(w64devkit))
    {
        found << StorageItem{QFileInfo(w64devkit).fileName(), s_pc, w64devkit, StorageItem::Origin::ProjectManager,
                             WindowsToolchain::w64devkitInstalled()};
    }

    appendSdk(found, QStringLiteral("VitaSDK"), QStringLiteral("PlayStation Vita"), qEnvironmentVariable("VITASDK"));
    appendSdk(found, QStringLiteral("devkitPro"), QStringLiteral("Nintendo Switch and Nintendo 3DS"),
              qEnvironmentVariable("DEVKITPRO"));
    appendSdk(found, QStringLiteral("fxSDK"), QStringLiteral("Casio fx-CG50"), fxsdkDirectory());

    return found;
}

// A link's target is measured where it lives, if it lives in here at all.
qint64 Storage::sizeOf(const QString &path)
{
    qint64 size = 0;
    QDirIterator files(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (files.hasNext())
    {
        const QFileInfo file = files.nextFileInfo();
        if (!file.isSymLink())
        {
            size += file.size();
        }
    }
    return size;
}
