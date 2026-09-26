#pragma once

#include "platform.h"

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

    static QString defaultDirectory(Platform platform);

    static QString name(Type type);
    static Type typeNamed(const QString &name);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(Platform platform, Type type, const QString &buildDirectory, QString *error);
};
