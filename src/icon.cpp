#include "icon.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QSaveFile>
#include <QSet>

#include <list>

namespace
{
    const QString s_pcPath = QStringLiteral("assets/icon.png");
    const QString s_vitaPath = QStringLiteral("sce_sys/icon0.png");
    const QString s_switchPath = QStringLiteral("switch/icon.jpg");
    const QString s_nintendo3dsPath = QStringLiteral("3ds/icon.png");

    constexpr int s_vitaIconSize = 128;
    constexpr int s_switchIconSize = 256;
    constexpr int s_nintendo3dsIconSize = 48;
    constexpr int s_largestWindowIcon = 512;
    constexpr int s_paletteColourLimit = 256;

    const char *formatOf(Platform platform)
    {
        return platform == Platform::Switch ? "jpg" : "png";
    }

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

    // A console icon is one fixed size, so the source is always resized to it.
    QImage resizedTo(const QImage &square, int size, QStringList *notes)
    {
        if (square.width() < size)
        {
            *notes << QStringLiteral("Enlarged from %1x%1, so it will look soft.").arg(square.width());
        }
        else if (square.width() > size)
        {
            *notes << QStringLiteral("Reduced from %1x%1.").arg(square.width());
        }

        return square.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
                if (seen.size() > s_paletteColourLimit)
                {
                    return seen.size();
                }
            }
        }
        return seen.size();
    }

    QImage forPc(const QImage &source, QStringList *notes)
    {
        QImage pc = squared(source, notes);
        if (pc.width() > s_largestWindowIcon)
        {
            *notes << QStringLiteral("Reduced from %1x%1 to %2x%2.").arg(pc.width()).arg(s_largestWindowIcon);
            pc = pc.scaled(s_largestWindowIcon, s_largestWindowIcon, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
        return pc.convertToFormat(pc.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    }

    QImage forVita(const QImage &source, QStringList *notes)
    {
        const QImage resized = resizedTo(squared(source, notes), s_vitaIconSize, notes);
        if (distinctColours(resized) > s_paletteColourLimit)
        {
            *notes << QStringLiteral("Approximated to %1 colours, which is all a palette PNG holds.").arg(s_paletteColourLimit);
        }

        // Qt keeps every colour of an image that has at most 256 of them, and approximates the rest.
        return resized.convertToFormat(QImage::Format_Indexed8, Qt::AutoColor | Qt::ThresholdDither);
    }

    // Composited rather than converted, so a transparent pixel becomes black
    // instead of whatever colour happened to sit underneath it.
    QImage flattenedOntoBlack(const QImage &image, const QString &reason, QStringList *notes)
    {
        if (image.hasAlphaChannel())
        {
            *notes << QStringLiteral("Flattened onto black, since %1.").arg(reason);
        }

        QImage flattened(image.size(), QImage::Format_RGB32);
        flattened.fill(Qt::black);
        QPainter painter(&flattened);
        painter.drawImage(0, 0, image);
        painter.end();
        return flattened;
    }

    QImage forSwitch(const QImage &source, QStringList *notes)
    {
        const QImage resized = resizedTo(squared(source, notes), s_switchIconSize, notes);
        return flattenedOntoBlack(resized, QStringLiteral("a JPEG holds no transparency"), notes);
    }

    QImage forNintendo3ds(const QImage &source, QStringList *notes)
    {
        const QImage resized = resizedTo(squared(source, notes), s_nintendo3dsIconSize, notes);
        return flattenedOntoBlack(resized, QStringLiteral("the 3DS stores its icon without transparency"), notes);
    }

    QString describeOne(Platform platform, const QImage &image)
    {
        if (platform == Platform::Vita)
        {
            return QStringLiteral("%1 a %2x%2 palette PNG of %3 colours")
                .arg(Icon::path(platform))
                .arg(image.width())
                .arg(image.colorCount());
        }

        if (platform == Platform::Switch)
        {
            return QStringLiteral("%1 a %2x%2 JPEG").arg(Icon::path(platform)).arg(image.width());
        }

        if (platform == Platform::Nintendo3ds)
        {
            return QStringLiteral("%1 a %2x%2 PNG").arg(Icon::path(platform)).arg(image.width());
        }

        return QStringLiteral("%1 %2x%2 truecolor").arg(Icon::path(platform)).arg(image.width());
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

QString Icon::path(Platform platform)
{
    if (platform == Platform::Vita)
    {
        return s_vitaPath;
    }

    if (platform == Platform::Switch)
    {
        return s_switchPath;
    }

    if (platform == Platform::Nintendo3ds)
    {
        return s_nintendo3dsPath;
    }

    return s_pcPath;
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

QImage Icon::render(Platform platform, const QImage &source, QStringList *notes)
{
    if (source.isNull())
    {
        return source;
    }

    if (platform == Platform::Vita)
    {
        return forVita(source, notes);
    }

    if (platform == Platform::Switch)
    {
        return forSwitch(source, notes);
    }

    if (platform == Platform::Nintendo3ds)
    {
        return forNintendo3ds(source, notes);
    }

    return forPc(source, notes);
}

bool Icon::write(const QString &projectDirectory, const QMap<Platform, QImage> &icons, QString *error)
{
    const QDir directory(projectDirectory);

    // Every file is staged before any of them is committed, so a failure replaces none.
    std::list<QSaveFile> staged;
    for (auto icon = icons.constBegin(); icon != icons.constEnd(); ++icon)
    {
        if (icon.value().isNull())
        {
            continue;
        }

        QSaveFile &file = staged.emplace_back(directory.filePath(path(icon.key())));
        if (!stage(file, icon.value(), formatOf(icon.key()), error))
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

QString Icon::describe(const QMap<Platform, QImage> &icons)
{
    QStringList written;
    for (auto icon = icons.constBegin(); icon != icons.constEnd(); ++icon)
    {
        if (!icon.value().isNull())
        {
            written << describeOne(icon.key(), icon.value());
        }
    }

    if (written.isEmpty())
    {
        return QStringLiteral("Nothing was written.");
    }

    // Only the first clause carries the verb, so a single file still reads as a sentence.
    written.first().insert(written.first().indexOf(QChar(' ')), QStringLiteral(" is now"));
    return written.join(QStringLiteral(", and ")) + QChar('.');
}
