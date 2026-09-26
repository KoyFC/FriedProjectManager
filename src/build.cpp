#include "build.h"

#include <QDir>
#include <QFileInfo>

namespace
{
    // One shape for both platforms; only the build tree and the toolchain differ.
    QList<QStringList> pipeline(const QString &buildTree, const QStringList &configureExtras)
    {
        QStringList configure = {QStringLiteral("cmake"), QStringLiteral("-S"), QStringLiteral("."),
                                 QStringLiteral("-B"), buildTree};
        configure += configureExtras;

        return {
            {QStringLiteral("haxe"), QStringLiteral("build.hxml")},
            configure,
            {QStringLiteral("cmake"), QStringLiteral("--build"), buildTree, QStringLiteral("--parallel")},
        };
    }
}

QString Build::defaultDirectory(int platform)
{
    return platform == PlatformVita ? QStringLiteral("build/vita") : QStringLiteral("build");
}

QList<QStringList> Build::commands(int platform, const QString &buildDirectory, QString *error)
{
    if (buildDirectory.isEmpty())
    {
        *error = QStringLiteral("There is no build directory to build in.");
        return {};
    }

    if (platform == PlatformPc)
    {
        return pipeline(buildDirectory, {});
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

    return pipeline(buildDirectory, {QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=%1").arg(toolchain)});
}
