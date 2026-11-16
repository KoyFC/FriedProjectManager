#pragma once

#include "platform.h"

#include <fried/core/icon.h>

#include <QImage>
#include <QList>
#include <QString>
#include <QStringList>

// A project keeps one icon per platform, because a window icon is truecolor of any
// size while each console reads one fixed size in a format of its own. What each
// one is, and how it is made, is FriedCore's.
class Icon
{
public:
    static QString path(Platform platform);

    // What that platform's file would hold, saying in notes whatever the source
    // had to become. The source is whatever Qt can read.
    static fried::core::RenderedIcon render(Platform platform, const QImage &source, QStringList *notes);

    // A null icon leaves that platform's file as it is.
    static bool write(const QString &projectDirectory, const QList<fried::core::RenderedIcon> &icons, QString *error);
    static QString describe(const QList<fried::core::RenderedIcon> &icons);
};
