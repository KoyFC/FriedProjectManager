#pragma once

#include <QList>
#include <QString>
#include <QStringList>

enum Platform
{
    PlatformPc,
    PlatformVita
};

class Build
{
public:
    // Where a platform builds when nothing has been chosen for it.
    static QString defaultDirectory(int platform);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(int platform, const QString &buildDirectory, QString *error);
};
