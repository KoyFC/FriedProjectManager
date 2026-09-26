#pragma once

#include "platform.h"

#include <QList>
#include <QString>
#include <QStringList>

class Build
{
public:
    static QString defaultDirectory(Platform platform);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(Platform platform, const QString &buildDirectory, QString *error);
};
