#pragma once

#include "build_target.h"

#include <QList>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

class Build
{
public:
    enum class Type
    {
        Debug,
        Release
    };

    static QString defaultDirectory(BuildTarget target);

    static QString name(Type type);
    static Type typeNamed(const QString &name);

    // On Windows without Visual Studio, until the tool has downloaded a compiler of its own.
    static bool needsCompilerDownload(BuildTarget target);
    static QProcessEnvironment environment(BuildTarget target);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(BuildTarget target, Type type, const QString &projectDirectory,
                                       const QString &buildDirectory, QString *error);
};
