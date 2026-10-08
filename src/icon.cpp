#include "icon.h"

#include "image_conversion.h"

#include <QPainter>

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

    const char *formatOf(Platform platform)
    {
        return platform == Platform::Switch ? "jpg" : "png";
    }

    // An icon is square wherever it is shown, so an oblong source keeps its middle.
    QImage squared(const QImage &image, QStringList *notes)
    {
        return ImageConversion::cropped(image, QSize(1, 1), notes);
    }

    QImage resizedTo(const QImage &square, int size, QStringList *notes)
    {
        return ImageConversion::resized(square, QSize(size, size), notes);
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
        return ImageConversion::palette(resizedTo(squared(source, notes), s_vitaIconSize, notes), notes);
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
    QList<ImageConversion::File> files;
    for (auto icon = icons.constBegin(); icon != icons.constEnd(); ++icon)
    {
        files.append({path(icon.key()), icon.value(), formatOf(icon.key())});
    }
    return ImageConversion::write(projectDirectory, files, error);
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
