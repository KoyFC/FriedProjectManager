#include "build.h"

#include "android_sdk.h"
#include "emscripten_sdk.h"
#include "windows_toolchain.h"

namespace
{
    // What this machine has to build with.
    fried::core::Toolchains toolchains()
    {
        fried::core::ManagedTools managed;
        managed.m_emscriptenSdk = fromQt(EmscriptenSdk::managedDirectory());
        managed.m_w64devkit = fromQt(WindowsToolchain::w64devkitDirectory());
        managed.m_androidNdk = fromQt(AndroidSdk::ndkDirectory());
        managed.m_androidSdk = fromQt(AndroidSdk::sdkDirectory());
        managed.m_javaHome = fromQt(AndroidSdk::jdkDirectory());
        fried::core::Toolchains found = fried::core::detectToolchains(managed);
        found.m_visualStudioCmake = fried::core::visualStudioCmake();
        return found;
    }

    QStringList argumentsOf(const fried::core::BuildStep &step)
    {
        QStringList arguments;
        for (const std::string &argument : step.m_arguments)
        {
            arguments << toQt(argument);
        }
        return arguments;
    }
}

bool Build::needsEmscriptenDownload(BuildTarget target)
{
    return fried::core::Build::needsEmscriptenDownload(target, toolchains());
}

bool Build::needsAndroidDownload(BuildTarget target)
{
    return fried::core::Build::needsAndroidDownload(target, toolchains());
}

bool Build::hasJdk()
{
    return !toolchains().m_javaHome.empty();
}

bool Build::needsCompilerDownload(BuildTarget target)
{
    return fried::core::Build::needsCompilerDownload(target, toolchains());
}

QProcessEnvironment Build::environment(BuildTarget target)
{
#ifdef Q_OS_WIN
    if (target == BuildTarget::Pc && fried::core::visualStudioCmake().empty())
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
    return toQt(fried::core::Build::defaultDirectory(target));
}

QString Build::name(Type type)
{
    return toQt(fried::core::buildTypeName(type));
}

Build::Type Build::typeNamed(const QString &name)
{
    return fried::core::buildTypeNamed(fromQt(name));
}

QStringList Build::haxeCommand(Type type, bool logToFile)
{
    return argumentsOf(fried::core::Build::haxeStep(type, logToFile));
}

QList<QStringList> Build::commands(BuildTarget target, Type type, bool logToFile,
                                   const QString &projectDirectory, const QString &buildDirectory,
                                   QString *error)
{
    std::string reason;
    const std::vector<fried::core::BuildStep> steps = fried::core::Build::steps(
        target, type, logToFile, fromQt(projectDirectory), fromQt(buildDirectory), toolchains(), &reason);
    if (steps.empty())
    {
        *error = toQt(reason);
        return {};
    }

    QList<QStringList> commands;
    for (const fried::core::BuildStep &step : steps)
    {
        commands << argumentsOf(step);
    }
    return commands;
}
