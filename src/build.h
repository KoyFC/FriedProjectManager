#pragma once

#include "build_target.h"

#include <fried/core/build.h>

#include <QList>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

// What a build runs is FriedCore's. This is the tool's side of it: what the machine has, which only the tool knows on Windows, and Qt's types.
class Build
{
public:
    using Type = fried::core::BuildType;

    static QString defaultDirectory(BuildTarget target);

    static QString name(Type type);

    // Debug builds the Haxe side with -debug, which is what makes the engine log where each message came from.
    static QStringList haxeCommand(Type type, bool logToFile);
    static Type typeNamed(const QString &name);

    // On Windows without Visual Studio, until the tool has downloaded a compiler of its own.
    static bool needsCompilerDownload(BuildTarget target);
    // The web build, with no Emscripten SDK anywhere the tool looks.
    static bool needsEmscriptenDownload(BuildTarget target);
    // The Android build, missing the NDK, the SDK or a JDK.
    static bool needsAndroidDownload(BuildTarget target);
    // Whether a JDK was found, so the Android download leaves it out.
    static bool hasJdk();
    static QProcessEnvironment environment(BuildTarget target);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(BuildTarget target, Type type, bool logToFile,
                                       const QString &projectDirectory, const QString &buildDirectory,
                                       QString *error);
};
