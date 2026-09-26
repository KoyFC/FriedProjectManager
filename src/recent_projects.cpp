#include "recent_projects.h"

#include <QDir>
#include <QSettings>

namespace
{
    const QString s_pathsKey = QStringLiteral("recentProjects/paths");

    constexpr int s_mostRemembered = 20;
}

QStringList RecentProjects::paths()
{
    return QSettings().value(s_pathsKey).toStringList();
}

void RecentProjects::remember(const QString &directory)
{
    const QString path = QDir::cleanPath(directory);
    if (path.isEmpty())
    {
        return;
    }

    QStringList recent = paths();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > s_mostRemembered)
    {
        recent.removeLast();
    }

    QSettings().setValue(s_pathsKey, recent);
}

void RecentProjects::forget(const QString &directory)
{
    QStringList recent = paths();
    if (recent.removeAll(QDir::cleanPath(directory)) > 0)
    {
        QSettings().setValue(s_pathsKey, recent);
    }
}
