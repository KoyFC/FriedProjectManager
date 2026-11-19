#include "asset_check.h"

#include "core_bridge.h"

#include <fried/core/asset_check.h>

bool AssetCheck::appliesTo(Platform platform)
{
    return platform == Platform::Vita || platform == Platform::Nintendo3ds || platform == Platform::NintendoDs || platform == Platform::Cg50;
}

QList<AssetIssue> AssetCheck::validate(Platform platform, const QString &projectDirectory)
{
    // FriedCore names an asset the way a scene file does, "game/sprite.png", and this lists it from the assets folder.
    const QString prefix = QStringLiteral("game/");

    QList<AssetIssue> issues;
    for (const fried::core::AssetIssue &found : fried::core::validateAssets(platform, fromQt(projectDirectory)))
    {
        QString asset = toQt(found.m_asset);
        if (asset.startsWith(prefix))
        {
            asset = asset.mid(prefix.size());
        }
        issues.append({found.m_severity == fried::core::AssetIssue::Severity::Error, asset, toQt(found.m_message)});
    }
    return issues;
}
