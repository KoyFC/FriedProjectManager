#pragma once

#include <QString>

enum class Platform
{
    Pc,
    Vita,
    Switch,
    Nintendo3ds
};

constexpr int s_platformCount = 4;

// The same name the engine gives each platform's section of project.fried.
inline QString platformKey(Platform platform)
{
    switch (platform)
    {
    case Platform::Vita:
        return QStringLiteral("vita");
    case Platform::Switch:
        return QStringLiteral("switch");
    case Platform::Nintendo3ds:
        return QStringLiteral("3ds");
    default:
        return QStringLiteral("pc");
    }
}
