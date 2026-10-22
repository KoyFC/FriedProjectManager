#pragma once

#include <QString>

// Where a kind of file dialog was last left, so the next one of that kind opens there.
class LastLocation
{
public:
    enum class Kind
    {
        Projects,
        Images
    };

    // The fallback when nothing was remembered or the folder is gone.
    static QString of(Kind kind, const QString &fallback);

    static void rememberFolderOf(Kind kind, const QString &file);

    // A chosen folder is remembered by what holds it, since it is a sibling that is usually wanted next.
    static void rememberParentOf(Kind kind, const QString &folder);
};
