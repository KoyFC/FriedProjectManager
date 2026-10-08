#include "image_conversion.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QSaveFile>
#include <QSet>

#include <list>

namespace
{
    constexpr int s_paletteColourLimit = 256;

    int distinctColours(const QImage &image)
    {
        const QImage rgb = image.convertToFormat(QImage::Format_ARGB32);
        QSet<QRgb> seen;
        for (int y = 0; y < rgb.height(); ++y)
        {
            for (int x = 0; x < rgb.width(); ++x)
            {
                seen.insert(rgb.pixel(x, y));
                if (seen.size() > s_paletteColourLimit)
                {
                    return seen.size();
                }
            }
        }
        return seen.size();
    }

    bool stage(QSaveFile &file, const QImage &image, const char *format, QString *error)
    {
        const QString path = QDir::toNativeSeparators(file.fileName());
        if (!QDir().mkpath(QFileInfo(file.fileName()).path()))
        {
            *error = QStringLiteral("Could not create the directory for %1.").arg(path);
            return false;
        }

        if (!file.open(QIODevice::WriteOnly))
        {
            *error = QStringLiteral("Could not write %1: %2").arg(path, file.errorString());
            return false;
        }

        QImageWriter writer(&file, format);
        if (!writer.write(image))
        {
            *error = QStringLiteral("Could not encode %1: %2").arg(path, writer.errorString());
            return false;
        }
        return true;
    }

    bool commit(QSaveFile &file, QString *error)
    {
        if (file.commit())
        {
            return true;
        }

        *error = QStringLiteral("Could not write %1: %2").arg(QDir::toNativeSeparators(file.fileName()), file.errorString());
        return false;
    }
}

QImage ImageConversion::cropped(const QImage &image, const QSize &shape, QStringList *notes)
{
    // Compared crosswise so no rounding decides whether the shapes match.
    const qint64 imageCross = qint64(image.width()) * shape.height();
    const qint64 shapeCross = qint64(shape.width()) * image.height();
    if (imageCross == shapeCross)
    {
        return image;
    }

    QSize kept = image.size();
    if (imageCross > shapeCross)
    {
        kept.setWidth(int(qint64(image.height()) * shape.width() / shape.height()));
    }
    else
    {
        kept.setHeight(int(qint64(image.width()) * shape.height() / shape.width()));
    }

    *notes << QStringLiteral("Uses the centre %1x%2 of %3x%4.")
                  .arg(kept.width())
                  .arg(kept.height())
                  .arg(image.width())
                  .arg(image.height());
    return image.copy((image.width() - kept.width()) / 2, (image.height() - kept.height()) / 2, kept.width(),
                      kept.height());
}

QImage ImageConversion::resized(const QImage &image, const QSize &size, QStringList *notes)
{
    if (image.width() < size.width())
    {
        *notes << QStringLiteral("Enlarged from %1x%2, so it will look soft.").arg(image.width()).arg(image.height());
    }
    else if (image.width() > size.width())
    {
        *notes << QStringLiteral("Reduced from %1x%2.").arg(image.width()).arg(image.height());
    }

    return image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QImage ImageConversion::palette(const QImage &image, QStringList *notes)
{
    if (distinctColours(image) > s_paletteColourLimit)
    {
        *notes << QStringLiteral("Approximated to %1 colours, which is all a palette PNG holds.").arg(s_paletteColourLimit);
    }

    // Qt keeps every colour of an image that has at most 256 of them, and approximates the rest.
    return image.convertToFormat(QImage::Format_Indexed8, Qt::AutoColor | Qt::ThresholdDither);
}

QImage ImageConversion::read(const QString &sourceImage, QString *error)
{
    QImageReader reader(sourceImage);

    // A photograph straight from a camera is stored rotated.
    reader.setAutoTransform(true);

    const QImage source = reader.read();
    if (source.isNull())
    {
        *error = QStringLiteral("%1 could not be read as an image: %2")
                     .arg(QFileInfo(sourceImage).fileName(), reader.errorString());
    }
    return source;
}

QStringList ImageConversion::readablePatterns()
{
    QStringList patterns;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
    {
        patterns << QStringLiteral("*.") + QString::fromUtf8(format);
    }
    return patterns;
}

bool ImageConversion::write(const QString &directory, const QList<File> &files, QString *error)
{
    const QDir root(directory);

    std::list<QSaveFile> staged;
    for (const File &file : files)
    {
        if (file.image.isNull())
        {
            continue;
        }

        QSaveFile &saved = staged.emplace_back(root.filePath(file.path));
        if (!stage(saved, file.image, file.format, error))
        {
            return false;
        }
    }

    for (QSaveFile &file : staged)
    {
        if (!commit(file, error))
        {
            return false;
        }
    }

    return true;
}
