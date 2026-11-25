#include "android_sdk.h"

#include "core_bridge.h"

#include <fried/core/android.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

namespace
{
    struct Archive
    {
        QString url;
        QString fileName;
        QString checksumTool;
        QString checksum;
    };

    const Archive s_ndk = {QStringLiteral("https://dl.google.com/android/repository/android-ndk-r27d-linux.zip"),
                           QStringLiteral("android-ndk-r27d-linux.zip"), QStringLiteral("sha1sum"),
                           QStringLiteral("22105e410cf29afcf163760cc95522b9fb981121")};
    const Archive s_platform = {QStringLiteral("https://dl.google.com/android/repository/platform-35_r02.zip"),
                                QStringLiteral("platform-35_r02.zip"), QStringLiteral("sha1sum"),
                                QStringLiteral("0bb560a90a7a2cbd0dd8348224d518b638fe7949")};
    const Archive s_buildTools = {QStringLiteral("https://dl.google.com/android/repository/build-tools_r35.0.1_linux.zip"),
                                  QStringLiteral("build-tools_r35.0.1_linux.zip"), QStringLiteral("sha1sum"),
                                  QStringLiteral("e009a9b188cfeb1d2b4c318ab5cb4f1ddc368861")};
    const Archive s_jdk = {QStringLiteral("https://github.com/adoptium/temurin21-binaries/releases/download/jdk-21.0.13.1%2B2/"
                                          "OpenJDK21U-jdk_x64_linux_hotspot_21.0.13.1_2.tar.gz"),
                           QStringLiteral("OpenJDK21U-jdk_x64_linux_hotspot_21.0.13.1_2.tar.gz"), QStringLiteral("sha256sum"),
                           QStringLiteral("bb8a93f9ae6e7a693b2b721e85dbeff26ae1a77447985a8d7afcdd9c7cf6222e")};

    // The folder each archive unpacks to, which is not always named after what it holds: build-tools 35.0.1 unpacks to android-15.
    const QString s_ndkFolder = QStringLiteral("android-ndk-r27d");
    const QString s_platformFolder = QStringLiteral("android-35");
    const QString s_buildToolsFolder = QStringLiteral("android-15");
    const QString s_jdkFolder = QStringLiteral("jdk-21.0.13.1+2");

    QString toolsDirectory()
    {
        return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath(QStringLiteral("tools"));
    }

    QString unpackDirectory()
    {
        return QDir(toolsDirectory()).filePath(QStringLiteral("android-unpacking"));
    }

    QString downloadPath(const Archive &archive)
    {
        return QDir(unpackDirectory()).filePath(archive.fileName);
    }

    QList<QStringList> fetch(const Archive &archive)
    {
        const QString file = downloadPath(archive);
        return {
            {QStringLiteral("curl"), QStringLiteral("--fail"), QStringLiteral("--location"), QStringLiteral("--retry"),
             QStringLiteral("3"), QStringLiteral("--output"), file, archive.url},
            {QStringLiteral("sh"), QStringLiteral("-c"),
             QStringLiteral("echo \"$1  $2\" | %1 -c -").arg(archive.checksumTool), QStringLiteral("sh"), archive.checksum, file},
        };
    }

    // A rename across the same disk, which every part is on, so nothing is copied.
    bool moveInto(const QString &unpacked, const QString &destination)
    {
        QDir(destination).removeRecursively();
        QDir().mkpath(QFileInfo(destination).absolutePath());
        return QDir().rename(QDir(unpackDirectory()).filePath(unpacked), destination);
    }
}

QString AndroidSdk::sdkDirectory()
{
    return toQt(fried::core::android::managedSdkDirectory(fromQt(toolsDirectory())));
}

QString AndroidSdk::ndkDirectory()
{
    return toQt(fried::core::android::managedNdkDirectory(fromQt(toolsDirectory())));
}

QString AndroidSdk::jdkDirectory()
{
    return toQt(fried::core::android::managedJdkDirectory(fromQt(toolsDirectory())));
}

QList<QStringList> AndroidSdk::beginInstall(bool needsJdk)
{
    QDir(unpackDirectory()).removeRecursively();
    QDir().mkpath(unpackDirectory());

    QList<QStringList> commands;
    for (const Archive *archive : {&s_ndk, &s_platform, &s_buildTools})
    {
        commands << fetch(*archive);
        commands << QStringList{QStringLiteral("unzip"), QStringLiteral("-q"), QStringLiteral("-o"), downloadPath(*archive),
                                QStringLiteral("-d"), unpackDirectory()};
    }
    if (needsJdk)
    {
        commands << fetch(s_jdk);
        commands << QStringList{QStringLiteral("tar"), QStringLiteral("-xzf"), downloadPath(s_jdk), QStringLiteral("-C"), unpackDirectory()};
    }
    return commands;
}

bool AndroidSdk::finishInstall(bool unpacked, bool needsJdk, QString *error)
{
    const QString sdk = sdkDirectory();
    if (unpacked)
    {
        unpacked = moveInto(s_ndkFolder, ndkDirectory()) &&
                   moveInto(s_platformFolder, QDir(sdk).filePath(QStringLiteral("platforms/%1").arg(fried::core::android::s_platform))) &&
                   moveInto(s_buildToolsFolder, QDir(sdk).filePath(QStringLiteral("build-tools/%1").arg(fried::core::android::s_buildTools)));
        if (unpacked && needsJdk)
        {
            unpacked = moveInto(s_jdkFolder, jdkDirectory());
        }
    }
    QDir(unpackDirectory()).removeRecursively();

    const bool hasAll = unpacked && fried::core::android::isNdk(fromQt(ndkDirectory())) && fried::core::android::isSdk(fromQt(sdk)) &&
                        (!needsJdk || fried::core::android::isJdk(fromQt(jdkDirectory())));
    if (!hasAll)
    {
        *error = QStringLiteral("The Android tools could not be installed in %1.")
                     .arg(QDir::toNativeSeparators(QFileInfo(sdk).absolutePath()));
        return false;
    }
    return true;
}
