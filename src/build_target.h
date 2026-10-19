#pragma once

#include "platform.h"

#include <QList>
#include <QString>

// What the build bar compiles for. Windows from another system is a target of
// its own, while the game it produces is still the PC platform.
enum class BuildTarget
{
    Pc,
    WindowsFromHost,
    Vita,
    Switch,
    Nintendo3ds,
    Cg50
};

inline QList<BuildTarget> buildTargets()
{
#ifdef Q_OS_WIN
    return {BuildTarget::Pc, BuildTarget::Vita, BuildTarget::Switch, BuildTarget::Nintendo3ds, BuildTarget::Cg50};
#else
    return {BuildTarget::Pc,          BuildTarget::WindowsFromHost, BuildTarget::Vita, BuildTarget::Switch,
            BuildTarget::Nintendo3ds, BuildTarget::Cg50};
#endif
}

inline Platform platformOf(BuildTarget target)
{
    switch (target)
    {
    case BuildTarget::Vita:
        return Platform::Vita;
    case BuildTarget::Switch:
        return Platform::Switch;
    case BuildTarget::Nintendo3ds:
        return Platform::Nintendo3ds;
    case BuildTarget::Cg50:
        return Platform::Cg50;
    default:
        return Platform::Pc;
    }
}

inline QString buildTargetKey(BuildTarget target)
{
    return target == BuildTarget::WindowsFromHost ? QStringLiteral("windows") : platformKey(platformOf(target));
}

inline QString buildTargetName(BuildTarget target)
{
    return target == BuildTarget::WindowsFromHost ? QStringLiteral("Windows") : platformName(platformOf(target));
}
