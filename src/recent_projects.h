#pragma once

#include <QString>
#include <QStringList>

// The projects the tool has opened, most recent first.
class RecentProjects
{
public:
    static QStringList paths();
    static void remember(const QString &directory);
    static void forget(const QString &directory);
};
