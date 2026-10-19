#pragma once

#include "build_target.h"

#include <QList>
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

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(BuildTarget target, Type type, const QString &projectDirectory,
                                       const QString &buildDirectory, QString *error);
};
