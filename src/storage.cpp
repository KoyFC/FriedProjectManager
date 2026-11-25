#include "storage.h"

#include "editor_install.h"
#include "android_sdk.h"
#include "core_bridge.h"
#include "emscripten_sdk.h"
#include "windows_toolchain.h"

#include <fried/core/android.h>

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

    const QString emsdk = EmscriptenSdk::managedDirectory();
    if (QFileInfo::exists(emsdk))
    {
        found << StorageItem{QFileInfo(emsdk).fileName(), QStringLiteral("Web"), emsdk,
                             StorageItem::Origin::ProjectManager, QFileInfo::exists(EmscriptenSdk::toolchainFile(emsdk))};
    }
    const QString androidSdk = AndroidSdk::sdkDirectory();
    if (QFileInfo::exists(androidSdk))
    {
        found << StorageItem{QStringLiteral("Android NDK and SDK"), QStringLiteral("Android"), androidSdk,
                             StorageItem::Origin::ProjectManager,
                             fried::core::android::isNdk(fromQt(AndroidSdk::ndkDirectory())) && fried::core::android::isSdk(fromQt(androidSdk))};
    }
    const QString jdk = AndroidSdk::jdkDirectory();
    if (QFileInfo::exists(jdk))
    {
        found << StorageItem{QStringLiteral("JDK 21"), QStringLiteral("Android"), jdk, StorageItem::Origin::ProjectManager,
                             fried::core::android::isJdk(fromQt(jdk))};
    }

    // Each version of Fried Editor has a folder of its own, and one still being unpacked is named after the version it will be.
    const QDir editors(EditorInstall::versionsDirectory());
    for (const QFileInfo &folder : editors.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        QString version = folder.fileName();
        const bool unpacking = version.endsWith(QStringLiteral("-unpacking"));
        if (unpacking)
        {
            version.chop(QStringLiteral("-unpacking").size());
        }
        found << StorageItem{QStringLiteral("Fried Editor %1").arg(version), QStringLiteral("Fried Editor"),
                             folder.absoluteFilePath(), StorageItem::Origin::ProjectManager,
                             !unpacking && EditorInstall::isInstalled(version)};
    }

    appendSdk(found, QStringLiteral("Emscripten SDK"), QStringLiteral("Web"), qEnvironmentVariable("EMSDK"));

    appendSdk(found, QStringLiteral("Android SDK"), QStringLiteral("Android"), qEnvironmentVariable("ANDROID_HOME"));
    appendSdk(found, QStringLiteral("Android NDK"), QStringLiteral("Android"), qEnvironmentVariable("ANDROID_NDK_ROOT"));

    appendSdk(found, QStringLiteral("VitaSDK"), QStringLiteral("PlayStation Vita"), qEnvironmentVariable("VITASDK"));
    appendSdk(found, QStringLiteral("devkitPro"), QStringLiteral("Nintendo Switch, Nintendo 3DS and Nintendo DS"),
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
