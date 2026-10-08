#pragma once

#include <QImage>
#include <QSize>
#include <QString>
#include <QStringList>

// The images a Vita shows on its home screen for a game, under sce_sys/livearea.
namespace LiveArea
{
    enum class Image
    {
        Background,
        Startup
    };

    QString path(Image image);
    QSize size(Image image);
    QString title(Image image);
    QString description(Image image);

    // What the file would hold, saying in notes whatever the source had to become.
    QImage render(Image image, const QImage &source, QStringList *notes);
}
