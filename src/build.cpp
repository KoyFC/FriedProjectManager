#include "build.h"

#include <QDir>
#include <QFileInfo>

namespace
{
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
    return platform == Platform::Vita ? QStringLiteral("build/vita") : QStringLiteral("build");
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

    const QString vitasdk = qEnvironmentVariable("VITASDK");
    if (vitasdk.isEmpty())
    {
        *error = QStringLiteral("VITASDK is not set, so there is no Vita toolchain to build with.");
        return {};
    }

    const QString toolchain = QDir(vitasdk).filePath(QStringLiteral("share/vita.toolchain.cmake"));
    if (!QFileInfo::exists(toolchain))
    {
        *error = QStringLiteral("VITASDK points at %1, where there is no share/vita.toolchain.cmake.")
                     .arg(QDir::toNativeSeparators(vitasdk));
        return {};
    }

    return pipeline(buildDirectory, type, {QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=%1").arg(toolchain)});
}
