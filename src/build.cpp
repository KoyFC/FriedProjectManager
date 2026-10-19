#include "build.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace
{
    struct ConsoleToolchain
    {
        const char *variable;
        QString relativePath;
        QString console;
    };

    ConsoleToolchain toolchainOf(Platform platform)
    {
        if (platform == Platform::Vita)
        {
            return {"VITASDK", QStringLiteral("share/vita.toolchain.cmake"), QStringLiteral("Vita")};
        }

        if (platform == Platform::Switch)
        {
            return {"DEVKITPRO", QStringLiteral("cmake/Switch.cmake"), QStringLiteral("Switch")};
        }

        return {"DEVKITPRO", QStringLiteral("cmake/3DS.cmake"), QStringLiteral("3DS")};
    }

    // Empty when the SDK is not installed or not where its variable points. Both
    // SDKs export that variable from a shell profile, so a manager started from a
    // desktop menu may never have inherited it.
    QString consoleToolchainFile(Platform platform, QString *error)
    {
        const ConsoleToolchain toolchain = toolchainOf(platform);

        const QString sdk = qEnvironmentVariable(toolchain.variable);
        if (sdk.isEmpty())
        {
            *error = QStringLiteral("%1 is not set, so there is no %2 toolchain to build with.")
                         .arg(QString::fromLatin1(toolchain.variable), toolchain.console);
            return {};
        }

        const QString file = QDir(sdk).filePath(toolchain.relativePath);
        if (!QFileInfo::exists(file))
        {
            *error = QStringLiteral("%1 points at %2, where there is no %3.")
                         .arg(QString::fromLatin1(toolchain.variable), QDir::toNativeSeparators(sdk),
                              toolchain.relativePath);
            return {};
        }

        return file;
    }

    // The fxSDK exports no variable of its own. It installs the fxsdk command into
    // <prefix>/bin and its CMake files under <prefix>/lib, so the command on the
    // PATH is what leads to the toolchain.
    QString fxsdkToolchainFile(QString *error)
    {
        const QString fxsdk = QStandardPaths::findExecutable(QStringLiteral("fxsdk"));
        if (fxsdk.isEmpty())
        {
            *error = QStringLiteral("fxsdk is not on the PATH, so there is no fx-CG50 toolchain to build with.");
            return {};
        }

        const QString file =
            QDir::cleanPath(QFileInfo(fxsdk).absoluteDir().filePath(QStringLiteral("../lib/cmake/fxsdk/FXCG50.cmake")));
        if (!QFileInfo::exists(file))
        {
            *error = QStringLiteral("%1 has no fx-CG50 toolchain at %2.")
                         .arg(QDir::toNativeSeparators(fxsdk), QDir::toNativeSeparators(file));
            return {};
        }

        return file;
    }

    // The engine ships this toolchain itself, so only the cross compiler has to be installed.
    QString mingwToolchainFile(const QString &projectDirectory, QString *error)
    {
        if (QStandardPaths::findExecutable(QStringLiteral("x86_64-w64-mingw32-g++")).isEmpty())
        {
            *error = QStringLiteral("x86_64-w64-mingw32-g++ is not on the PATH, so there is no MinGW-w64 compiler to "
                                    "build for Windows with.");
            return {};
        }

        const QString file = QDir(projectDirectory).filePath(QStringLiteral("engine/cmake/toolchains/mingw-w64.cmake"));
        if (!QFileInfo::exists(file))
        {
            *error = QStringLiteral("The project's engine has no MinGW-w64 toolchain at %1. Updating the engine "
                                    "submodule brings it.")
                         .arg(QDir::toNativeSeparators(file));
            return {};
        }

        return file;
    }

    QList<QStringList> pipeline(const QString &buildTree, Build::Type type, const QStringList &configureExtras)
    {
        const QString typeName = Build::name(type);

        QStringList configure = {QStringLiteral("cmake"), QStringLiteral("-S"), QStringLiteral("."),
                                 QStringLiteral("-B"), buildTree,
                                 QStringLiteral("-DCMAKE_BUILD_TYPE=%1").arg(typeName)};
        configure += configureExtras;

        return {
            {QStringLiteral("haxe"), QStringLiteral("build.hxml")},
            configure,
            // A generator that holds every configuration at once ignores the variable and reads --config
            {QStringLiteral("cmake"), QStringLiteral("--build"), buildTree, QStringLiteral("--config"), typeName,
             QStringLiteral("--parallel")},
        };
    }
}

QString Build::defaultDirectory(BuildTarget target)
{
    if (target == BuildTarget::Pc)
    {
        return QStringLiteral("build");
    }

    return QStringLiteral("build/%1").arg(buildTargetKey(target));
}

QString Build::name(Type type)
{
    return type == Type::Release ? QStringLiteral("Release") : QStringLiteral("Debug");
}

Build::Type Build::typeNamed(const QString &name)
{
    return name == Build::name(Type::Release) ? Type::Release : Type::Debug;
}

QList<QStringList> Build::commands(BuildTarget target, Type type, const QString &projectDirectory,
                                   const QString &buildDirectory, QString *error)
{
    if (buildDirectory.isEmpty())
    {
        *error = QStringLiteral("There is no build directory to build in.");
        return {};
    }

    QString toolchain;
    switch (target)
    {
    case BuildTarget::Pc:
        return pipeline(buildDirectory, type, {});
    case BuildTarget::WindowsFromHost:
        toolchain = mingwToolchainFile(projectDirectory, error);
        break;
    case BuildTarget::Cg50:
        toolchain = fxsdkToolchainFile(error);
        break;
    default:
        toolchain = consoleToolchainFile(platformOf(target), error);
    }
    if (toolchain.isEmpty())
    {
        return {};
    }

    return pipeline(buildDirectory, type, {QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=%1").arg(toolchain)});
}
