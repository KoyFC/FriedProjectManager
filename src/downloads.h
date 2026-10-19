#pragma once

#include <QList>
#include <QString>

// What the engine's builds and this tool have downloaded outside any project,
// which deleting only costs a download the next time it is needed.
struct Download
{
    QString name;
    QString kind;
    QString path;
    bool complete = true;
};

namespace Downloads
{
    // The engine's default FRIED_DEPS_CACHE, computed the way cmake/PcDependencies.cmake does.
    QString dependencySourcesDirectory();

    QList<Download> installed();
    qint64 sizeOf(const QString &path);
}
