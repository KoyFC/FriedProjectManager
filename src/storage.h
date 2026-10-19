#pragma once

#include <QList>
#include <QString>

// Something outside every project that builds depend on and that takes space.
struct StorageItem
{
    enum class Origin
    {
        EngineBuilds,
        ProjectManager,
        ItsOwnInstaller
    };

    QString name;
    QString platform;
    QString path;
    Origin origin = Origin::EngineBuilds;
    bool complete = true;

    // Whatever was downloaded comes back on the next build that needs it. An
    // SDK belongs to the installer that put it there.
    bool removable() const { return origin != Origin::ItsOwnInstaller; }
};

namespace Storage
{
    // The engine's default FRIED_DEPS_CACHE, computed the way cmake/PcDependencies.cmake does.
    QString dependencySourcesDirectory();

    QList<StorageItem> items();
    qint64 sizeOf(const QString &path);
}
