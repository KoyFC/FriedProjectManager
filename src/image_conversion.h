#pragma once

#include <QImage>
#include <QList>
#include <QSize>
#include <QString>
#include <QStringList>

// What every file the tool writes from someone's image has in common. Each step
// says in notes whatever it had to do to the source.
namespace ImageConversion
{
    // Keeps the middle of the source, cut to the shape of the target size.
    QImage cropped(const QImage &image, const QSize &shape, QStringList *notes);

    // A console reads one fixed size, so the source is always resized to it.
    QImage resized(const QImage &image, const QSize &size, QStringList *notes);

    // The Vita's installer rejects a truecolor PNG wherever it reads one.
    QImage palette(const QImage &image, QStringList *notes);

    // Null when the file cannot be read as an image, with the reason in error.
    QImage read(const QString &sourceImage, QString *error);
    QStringList readablePatterns();

    struct File
    {
        QString path;
        QImage image;
        const char *format = "png";
    };

    // Every file is staged before any of them is committed, so a failure replaces
    // none. Paths are relative to the directory, and null images are skipped.
    bool write(const QString &directory, const QList<File> &files, QString *error);
}
