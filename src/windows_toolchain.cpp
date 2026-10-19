#include "windows_toolchain.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

namespace
{
    const QString s_w64devkitVersion = QStringLiteral("2.10.1");
    const QString s_w64devkitUrl =
        QStringLiteral("https://github.com/skeeto/w64devkit/releases/download/v2.10.1/w64devkit-x64-2.10.1.7z.exe");
    const QString s_w64devkitSha256 =
        QStringLiteral("add1edf984ce4d473cfeb7c52e0ab38db73a855bba6c7f564d5e16648eb2896a");

    QString toolsDirectory()
    {
        return QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
            .filePath(QStringLiteral("tools"));
    }

    QString downloadPath()
    {
        return QDir(toolsDirectory()).filePath(QStringLiteral("w64devkit-%1.7z.exe").arg(s_w64devkitVersion));
    }

    QString unpackDirectory()
    {
        return QDir(toolsDirectory()).filePath(QStringLiteral("w64devkit-%1-unpacking").arg(s_w64devkitVersion));
    }

    QString w64devkitTool(const QString &name)
    {
        return QDir::toNativeSeparators(
            QDir(WindowsToolchain::w64devkitDirectory()).filePath(QStringLiteral("bin/%1.exe").arg(name)));
    }

    QString visualStudioInstallation()
    {
        const QString vswhere = QDir(qEnvironmentVariable("ProgramFiles(x86)"))
                                    .filePath(QStringLiteral("Microsoft Visual Studio/Installer/vswhere.exe"));
        if (!QFileInfo::exists(vswhere))
        {
            return QString();
        }

        QProcess query;
        query.start(vswhere, {QStringLiteral("-latest"), QStringLiteral("-products"), QStringLiteral("*"),
                              QStringLiteral("-requires"),
                              QStringLiteral("Microsoft.VisualStudio.Component.VC.Tools.x86.x64"),
                              QStringLiteral("-property"), QStringLiteral("installationPath")});
        if (!query.waitForFinished() || query.exitCode() != 0)
        {
            return QString();
        }

        return QString::fromLocal8Bit(query.readAllStandardOutput()).trimmed();
    }
}

QString WindowsToolchain::visualStudioCmake()
{
    const QString installation = visualStudioInstallation();
    if (installation.isEmpty())
    {
        return QString();
    }

    const QString onPath = QStandardPaths::findExecutable(QStringLiteral("cmake"));
    if (!onPath.isEmpty())
    {
        return onPath;
    }

    // The CMake that Visual Studio's C++ workload installs for its own use.
    const QString bundled = QDir(installation).filePath(
        QStringLiteral("Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"));
    return QFileInfo::exists(bundled) ? QDir::toNativeSeparators(bundled) : QString();
}

QString WindowsToolchain::w64devkitDirectory()
{
    return QDir(toolsDirectory()).filePath(QStringLiteral("w64devkit-%1").arg(s_w64devkitVersion));
}

bool WindowsToolchain::w64devkitInstalled()
{
    for (const QString &tool : {QStringLiteral("cmake"), QStringLiteral("ninja"), QStringLiteral("g++")})
    {
        if (!QFileInfo::exists(w64devkitTool(tool)))
        {
            return false;
        }
    }
    return true;
}

QList<QStringList> WindowsToolchain::beginInstall()
{
    QDir().mkpath(toolsDirectory());
    QFile::remove(downloadPath());
    QDir(unpackDirectory()).removeRecursively();

    const QString download = QDir::toNativeSeparators(downloadPath());
    const QString hashCheck =
        QStringLiteral("if ((Get-FileHash -Algorithm SHA256 -LiteralPath '%1').Hash -ne '%2') "
                       "{ Write-Error 'The download is not the w64devkit %3 release.'; exit 1 }")
            .arg(download, s_w64devkitSha256.toUpper(), s_w64devkitVersion);

    return {
        {QStringLiteral("curl.exe"), QStringLiteral("--fail"), QStringLiteral("--location"),
         QStringLiteral("--retry"), QStringLiteral("3"), QStringLiteral("--output"), download, s_w64devkitUrl},
        {QStringLiteral("powershell.exe"), QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"),
         QStringLiteral("-Command"), hashCheck},
        // A 7-Zip self-extracting archive, which unpacks without asking when given -y.
        {download, QStringLiteral("-o%1").arg(QDir::toNativeSeparators(unpackDirectory())), QStringLiteral("-y")},
    };
}

bool WindowsToolchain::finishInstall(bool unpacked, QString *error)
{
    QFile::remove(downloadPath());

    if (unpacked)
    {
        QDir(w64devkitDirectory()).removeRecursively();
        unpacked = QDir().rename(QDir(unpackDirectory()).filePath(QStringLiteral("w64devkit")), w64devkitDirectory());
    }
    QDir(unpackDirectory()).removeRecursively();

    if (!unpacked || !w64devkitInstalled())
    {
        *error = QStringLiteral("w64devkit could not be installed in %1.")
                     .arg(QDir::toNativeSeparators(w64devkitDirectory()));
        return false;
    }

    return true;
}

QList<QStringList> WindowsToolchain::w64devkitPipeline(const QString &buildTree, const QString &buildType)
{
    const QString cmake = w64devkitTool(QStringLiteral("cmake"));

    return {
        {QStringLiteral("haxe"), QStringLiteral("build.hxml")},
        {cmake, QStringLiteral("-S"), QStringLiteral("."), QStringLiteral("-B"), buildTree, QStringLiteral("-G"),
         QStringLiteral("Ninja"), QStringLiteral("-DCMAKE_MAKE_PROGRAM=%1").arg(w64devkitTool(QStringLiteral("ninja"))),
         QStringLiteral("-DCMAKE_C_COMPILER=%1").arg(w64devkitTool(QStringLiteral("gcc"))),
         QStringLiteral("-DCMAKE_CXX_COMPILER=%1").arg(w64devkitTool(QStringLiteral("g++"))),
         QStringLiteral("-DCMAKE_BUILD_TYPE=%1").arg(buildType)},
        {cmake, QStringLiteral("--build"), buildTree, QStringLiteral("--parallel")},
    };
}

QProcessEnvironment WindowsToolchain::w64devkitEnvironment()
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString bin = QDir::toNativeSeparators(QDir(w64devkitDirectory()).filePath(QStringLiteral("bin")));
    environment.insert(QStringLiteral("PATH"), bin + QDir::listSeparator() + environment.value(QStringLiteral("PATH")));
    return environment;
}
