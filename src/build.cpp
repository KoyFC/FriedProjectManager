#include "build.h"

#include "windows_toolchain.h"

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

    // A game holds the engine as its engine/ submodule, while the engine's own
    // sandbox sits inside the engine. Empty when neither is there.
    QString engineDirectoryOf(const QString &projectDirectory)
    {
        const QDir project(projectDirectory);
        for (const QString &candidate : {project.filePath(QStringLiteral("engine")), project.filePath(QStringLiteral(".."))})
        {
            if (QFileInfo::exists(QDir(candidate).filePath(QStringLiteral("cmake/FriedEngine.cmake"))))
            {
                return QDir::cleanPath(candidate);
            }
        }

        return {};
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

        const QString engine = engineDirectoryOf(projectDirectory);
        if (engine.isEmpty())
        {
            *error = QStringLiteral("There is no Fried Engine in the project's engine/ submodule, so there is no "
                                    "MinGW-w64 toolchain to build for Windows with.");
            return {};
        }

        const QString file = QDir(engine).filePath(QStringLiteral("cmake/toolchains/mingw-w64.cmake"));
        if (!QFileInfo::exists(file))
        {
            *error = QStringLiteral("The project's engine has no MinGW-w64 toolchain at %1. Updating the engine "
                                    "submodule brings it.")
                         .arg(QDir::toNativeSeparators(file));
            return {};
        }

        return file;
    }

    QList<QStringList> pipeline(const QString &buildTree, Build::Type type, const QStringList &configureExtras,
                                const QString &cmake = QStringLiteral("cmake"))
    {
        const QString typeName = Build::name(type);

        QStringList configure = {cmake, QStringLiteral("-S"), QStringLiteral("."),
                                 QStringLiteral("-B"), buildTree,
                                 QStringLiteral("-DCMAKE_BUILD_TYPE=%1").arg(typeName)};
        configure += configureExtras;

        return {
            {QStringLiteral("haxe"), QStringLiteral("build.hxml")},
            configure,
            // A generator that holds every configuration at once ignores the variable and reads --config
            {cmake, QStringLiteral("--build"), buildTree, QStringLiteral("--config"), typeName,
             QStringLiteral("--parallel")},
        };
    }

#ifdef Q_OS_WIN
    QList<QStringList> windowsPipeline(const QString &buildTree, Build::Type type, QString *error)
    {
        const QString visualStudioCmake = WindowsToolchain::visualStudioCmake();
        if (!visualStudioCmake.isEmpty())
        {
            return pipeline(buildTree, type, {}, visualStudioCmake);
        }

        if (WindowsToolchain::w64devkitInstalled())
        {
            return WindowsToolchain::w64devkitPipeline(buildTree, Build::name(type));
        }

        *error = QStringLiteral("There is no C++ compiler to build with: Visual Studio's C++ tools are not "
                                "installed, and neither is w64devkit.");
        return {};
    }
#endif
}

bool Build::needsCompilerDownload(BuildTarget target)
{
#ifdef Q_OS_WIN
    return target == BuildTarget::Pc && WindowsToolchain::visualStudioCmake().isEmpty()
           && !WindowsToolchain::w64devkitInstalled();
#else
    Q_UNUSED(target);
    return false;
#endif
}

QProcessEnvironment Build::environment(BuildTarget target)
{
#ifdef Q_OS_WIN
    if (target == BuildTarget::Pc && WindowsToolchain::visualStudioCmake().isEmpty())
    {
        return WindowsToolchain::w64devkitEnvironment();
    }
#else
    Q_UNUSED(target);
#endif
    return QProcessEnvironment::systemEnvironment();
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
#ifdef Q_OS_WIN
        return windowsPipeline(buildDirectory, type, error);
#else
        return pipeline(buildDirectory, type, {});
#endif
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
