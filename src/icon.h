#pragma once

#include "platform.h"

#include <QImage>
#include <QMap>
#include <QString>
#include <QStringList>

// A project keeps one icon per platform, because a window icon is truecolor of any
// size while each console reads one fixed size in a format of its own.
class Icon
{
public:
    static QString path(Platform platform);

    // What that platform's file would hold, saying in notes whatever the source
    // had to become.
    static QImage render(Platform platform, const QImage &source, QStringList *notes);

    // A null image leaves that platform's file as it is.
    static bool write(const QString &projectDirectory, const QMap<Platform, QImage> &icons, QString *error);
    static QString describe(const QMap<Platform, QImage> &icons);
};
