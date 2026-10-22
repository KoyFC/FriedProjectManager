#include "emscripten_sdk.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace
{
    const QString s_version = QStringLiteral("6.0.13");
    const QString s_emsdkUrl = QStringLiteral("https://github.com/emscripten-core/emsdk.git");

    QString emsdkCommand()
    {
#ifdef Q_OS_WIN
        return QStringLiteral("emsdk.bat");
#else
        return QStringLiteral("emsdk");
#endif
    }

    // A command line that runs the emsdk script in the managed folder with these arguments.
    QStringList emsdk(const QStringList &arguments)
    {
        const QString script = QDir::toNativeSeparators(QDir(EmscriptenSdk::managedDirectory()).filePath(emsdkCommand()));
#ifdef Q_OS_WIN
        return QStringList{QStringLiteral("cmd.exe"), QStringLiteral("/c"), script} + arguments;
#else
        return QStringList{script} + arguments;
#endif
    }
}

QString EmscriptenSdk::managedDirectory()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("tools/emsdk"));
}

QString EmscriptenSdk::toolchainFile(const QString &directory)
{
    return QDir(directory).filePath(QStringLiteral("upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"));
}

QString EmscriptenSdk::directory()
{
    for (const QString &candidate : {qEnvironmentVariable("EMSDK"), managedDirectory(),
                                     QDir::home().filePath(QStringLiteral("emsdk"))})
    {
        if (!candidate.isEmpty() && QFileInfo::exists(toolchainFile(candidate)))
        {
            return candidate;
        }
    }
    return QString();
}

QList<QStringList> EmscriptenSdk::beginInstall()
{
    QDir(managedDirectory()).removeRecursively();
    QDir().mkpath(QFileInfo(managedDirectory()).absolutePath());

    return {
        {QStringLiteral("git"), QStringLiteral("clone"), QStringLiteral("--depth"), QStringLiteral("1"), s_emsdkUrl,
         QDir::toNativeSeparators(managedDirectory())},
        emsdk({QStringLiteral("install"), s_version}),
        emsdk({QStringLiteral("activate"), s_version}),
    };
}

bool EmscriptenSdk::finishInstall(bool installed, QString *error)
{
    if (installed && QFileInfo::exists(toolchainFile(managedDirectory())))
    {
        return true;
    }

    // What is left is half an SDK that would be skipped anyway, and taking space.
    QDir(managedDirectory()).removeRecursively();
    *error = QStringLiteral("The Emscripten SDK could not be installed in %1.")
                 .arg(QDir::toNativeSeparators(managedDirectory()));
    return false;
}
