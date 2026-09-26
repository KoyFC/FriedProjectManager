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

QList<QStringList> Build::commands(int platform, QString *error)
{
    if (platform == PlatformPc)
    {
        return pipeline(QStringLiteral("build"), {});
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

    return pipeline(QStringLiteral("build/vita"),
                    {QStringLiteral("-DCMAKE_TOOLCHAIN_FILE=%1").arg(toolchain)});
}
