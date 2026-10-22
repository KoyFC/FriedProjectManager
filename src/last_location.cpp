#include "last_location.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace
{
    QString key(LastLocation::Kind kind)
    {
        return QStringLiteral("lastLocations/%1").arg(kind == LastLocation::Kind::Images ? "images" : "projects");
    }

    void remember(LastLocation::Kind kind, const QString &directory)
    {
        if (!directory.isEmpty())
        {
            QSettings().setValue(key(kind), directory);
        }
    }
}

QString LastLocation::of(LastLocation::Kind kind, const QString &fallback)
{
    const QString remembered = QSettings().value(key(kind)).toString();
    return QDir(remembered).exists() && !remembered.isEmpty() ? remembered : fallback;
}

void LastLocation::rememberFolderOf(LastLocation::Kind kind, const QString &file)
{
    remember(kind, QFileInfo(file).absolutePath());
}

void LastLocation::rememberParentOf(LastLocation::Kind kind, const QString &folder)
{
    remember(kind, QDir::cleanPath(QDir(folder).filePath(QStringLiteral(".."))));
}
