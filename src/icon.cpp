#include "icon.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QSaveFile>
#include <QSet>

namespace
{
    const QString s_pcPath = QStringLiteral("assets/icon.png");
    const QString s_vitaPath = QStringLiteral("sce_sys/icon0.png");

    // What the console's installer accepts, and nothing else.
    constexpr int s_vitaSize = 128;

    // No desktop draws a window icon larger than this.
    constexpr int s_pcLimit = 512;

    // The most colours a palette PNG can hold.
    constexpr int s_paletteLimit = 256;

    // An icon is square wherever it is shown, so an oblong source keeps its middle.
    QImage squared(const QImage &image, QStringList *notes)
    {
        if (image.width() == image.height())
        {
            return image;
        }

        const int side = qMin(image.width(), image.height());
        *notes << QStringLiteral("Uses the centre %1x%1 square of %2x%3.").arg(side).arg(image.width()).arg(image.height());
        return image.copy((image.width() - side) / 2, (image.height() - side) / 2, side, side);
    }

    int distinctColours(const QImage &image)
    {
        const QImage rgb = image.convertToFormat(QImage::Format_ARGB32);
        QSet<QRgb> seen;
        for (int y = 0; y < rgb.height(); ++y)
        {
            for (int x = 0; x < rgb.width(); ++x)
            {
                seen.insert(rgb.pixel(x, y));
                if (seen.size() > s_paletteLimit)
                {
                    return seen.size();
                }
            }
        }
        return seen.size();
    }

    bool stage(QSaveFile &file, const QImage &image, QString *error)
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

        QImageWriter writer(&file, "png");
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

QString Icon::pcPath()
{
    return s_pcPath;
}

QString Icon::vitaPath()
{
    return s_vitaPath;
}

QStringList Icon::readablePatterns()
{
    QStringList patterns;
    for (const QByteArray &format : QImageReader::supportedImageFormats())
    {
        patterns << QStringLiteral("*.") + QString::fromUtf8(format);
    }
    return patterns;
}

QImage Icon::read(const QString &sourceImage, QString *error)
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

QImage Icon::forPc(const QImage &source, QStringList *notes)
{
    if (source.isNull())
    {
        return source;
    }

    QImage pc = squared(source, notes);
    if (pc.width() > s_pcLimit)
    {
        *notes << QStringLiteral("Reduced from %1x%1 to %2x%2.").arg(pc.width()).arg(s_pcLimit);
        pc = pc.scaled(s_pcLimit, s_pcLimit, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return pc.convertToFormat(pc.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
}

QImage Icon::forVita(const QImage &source, QStringList *notes)
{
    if (source.isNull())
    {
        return source;
    }

    const QImage square = squared(source, notes);
    if (square.width() < s_vitaSize)
    {
        *notes << QStringLiteral("Enlarged from %1x%1, so it will look soft.").arg(square.width());
    }
    else if (square.width() > s_vitaSize)
    {
        *notes << QStringLiteral("Reduced from %1x%1.").arg(square.width());
    }

    const QImage resized = square.scaled(s_vitaSize, s_vitaSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (distinctColours(resized) > s_paletteLimit)
    {
        *notes << QStringLiteral("Approximated to %1 colours, which is all a palette PNG holds.").arg(s_paletteLimit);
    }

    // Qt keeps every colour of an image that has at most 256 of them, and approximates the rest.
    return resized.convertToFormat(QImage::Format_Indexed8, Qt::AutoColor | Qt::ThresholdDither);
}

bool Icon::write(const QString &projectDirectory, const QImage &pc, const QImage &vita, QString *error)
{
    const QDir directory(projectDirectory);
    QSaveFile pcFile(directory.filePath(s_pcPath));
    QSaveFile vitaFile(directory.filePath(s_vitaPath));

    // Both are staged before either is committed, so a failure replaces neither.
    if (!pc.isNull() && !stage(pcFile, pc, error))
    {
        return false;
    }
    if (!vita.isNull() && !stage(vitaFile, vita, error))
    {
        return false;
    }

    if (!pc.isNull() && !commit(pcFile, error))
    {
        return false;
    }
    return vita.isNull() || commit(vitaFile, error);
}

QString Icon::describe(const QImage &pc, const QImage &vita)
{
    QStringList written;
    if (!pc.isNull())
    {
        written << QStringLiteral("%1 %2x%2 truecolor").arg(s_pcPath).arg(pc.width());
    }
    if (!vita.isNull())
    {
        written << QStringLiteral("%1 a %2x%2 palette PNG of %3 colours")
                       .arg(s_vitaPath)
                       .arg(vita.width())
                       .arg(vita.colorCount());
    }

    if (written.isEmpty())
    {
        return QStringLiteral("Nothing was written.");
    }

    // Only the first clause carries the verb, so a single file still reads as a sentence.
    written.first().insert(written.first().indexOf(QChar(' ')), QStringLiteral(" is now"));
    return written.join(QStringLiteral(", and ")) + QChar('.');
}
