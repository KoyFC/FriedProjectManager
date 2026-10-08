#pragma once

#include <QString>

enum class Platform
{
    Pc,
    Vita,
    Switch,
    Nintendo3ds,
    Cg50
};

constexpr int s_platformCount = 5;

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
    case Platform::Cg50:
        return QStringLiteral("cg50");
    default:
        return QStringLiteral("pc");
    }
}

inline QString platformName(Platform platform)
{
    switch (platform)
    {
    case Platform::Vita:
        return QStringLiteral("PlayStation Vita");
    case Platform::Switch:
        return QStringLiteral("Nintendo Switch");
    case Platform::Nintendo3ds:
        return QStringLiteral("Nintendo 3DS");
    case Platform::Cg50:
        return QStringLiteral("Casio fx-CG50");
    default:
        return QStringLiteral("PC");
    }
}
