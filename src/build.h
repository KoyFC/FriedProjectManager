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
    static QString defaultDirectory(int platform);

    // Empty when the platform cannot be built on this machine, with the reason in error.
    static QList<QStringList> commands(int platform, const QString &buildDirectory, QString *error);
};
