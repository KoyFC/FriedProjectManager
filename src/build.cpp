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

QString Build::defaultDirectory(Platform platform)
{
    if (platform == Platform::Vita)
    {
        return QStringLiteral("build/vita");
    }

    if (platform == Platform::Switch)
    {
        return QStringLiteral("build/switch");
    }

    if (platform == Platform::Nintendo3ds)
    {
        return QStringLiteral("build/3ds");
    }

    if (platform == Platform::Cg50)
    {
        return QStringLiteral("build/cg50");
    }

    return QStringLiteral("build");
}

QString Build::name(Type type)
{
    return type == Type::Release ? QStringLiteral("Release") : QStringLiteral("Debug");
}

Build::Type Build::typeNamed(const QString &name)
{
    return name == Build::name(Type::Release) ? Type::Release : Type::Debug;
}

QList<QStringList> Build::commands(Platform platform, Type type, const QString &buildDirectory, QString *error)
{
    if (buildDirectory.isEmpty())
    {
        *error = QStringLiteral("There is no build directory to build in.");
        return {};
    }

    if (platform == Platform::Pc)
    {
        return pipeline(buildDirectory, type, {});
    }

    const QString toolchain = platform == Platform::Cg50 ? fxsdkToolchainFile(error) : consoleToolchainFile(platform, error);
    if (toolchain.isEmpty())
    {
        return {};
    }

    return pipeline(buildDirectory, type, {QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=%1").arg(toolchain)});
}
