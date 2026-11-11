#pragma once

#include "platform.h"

#include <QList>
#include <QString>

// A way a project's asset does not suit a platform. The same limits as the Fried Editor's Issues tab, which this reports and does not fix.
struct AssetIssue
{
    bool isError = false;

    // Relative to the project's assets folder, or empty for something about the project as a whole.
    QString asset;
    QString message;
};

class AssetCheck
{
public:
    static bool appliesTo(Platform platform);

    // Checks the project's own assets, in assets/ under the project.
    static QList<AssetIssue> validate(Platform platform, const QString &projectDirectory);
};
