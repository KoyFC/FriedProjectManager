#include "recent_projects.h"

#include <QDir>
#include <QSettings>

namespace
{
    const QString s_key = QStringLiteral("recentProjects/paths");

    // Enough to cover everything worked on lately without the list needing a scrollbar.
    constexpr int s_limit = 20;
}

QStringList RecentProjects::paths()
{
    return QSettings().value(s_key).toStringList();
}

void RecentProjects::remember(const QString &directory)
{
    const QString path = QDir::cleanPath(directory);
    if (path.isEmpty())
    {
        return;
    }

    QStringList recent = paths();

    // Opening one again moves it up rather than listing it twice.
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > s_limit)
    {
        recent.removeLast();
    }

    QSettings().setValue(s_key, recent);
}

void RecentProjects::forget(const QString &directory)
{
    QStringList recent = paths();
    if (recent.removeAll(QDir::cleanPath(directory)) > 0)
    {
        QSettings().setValue(s_key, recent);
    }
}
