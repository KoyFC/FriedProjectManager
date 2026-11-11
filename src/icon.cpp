#include "icon.h"

#include "image_conversion.h"

#include <QColor>
#include <QPainter>

#include <algorithm>
#include <climits>
#include <vector>

namespace
{
    const QString s_pcPath = QStringLiteral("assets/icon.png");
    const QString s_vitaPath = QStringLiteral("sce_sys/icon0.png");
    const QString s_switchPath = QStringLiteral("switch/icon.jpg");
    const QString s_nintendo3dsPath = QStringLiteral("3ds/icon.png");
    const QString s_nintendoDsPath = QStringLiteral("nds/icon.bmp");
    const QString s_cg50Path = QStringLiteral("cg50/icon-uns.png");
    const QString s_cg50SelectedPath = QStringLiteral("cg50/icon-sel.png");

    constexpr int s_vitaIconSize = 128;
    constexpr int s_switchIconSize = 256;
    constexpr int s_nintendo3dsIconSize = 48;
    constexpr int s_nintendoDsIconSize = 32;
    // The DS menu's icon has sixteen colors, the first of them transparent.
    constexpr int s_nintendoDsOpaqueColors = 15;
    constexpr int s_largestWindowIcon = 512;

    // A .g3a's menu icon, drawn inside a frame of the calculator's own: white
    // around it unselected and blue selected, the way the fxSDK's template draws
    // its pair, so the menu shows which one is chosen.
    const QSize s_cg50IconSize(92, 64);
    constexpr int s_cg50FrameWidth = 5;
    constexpr qreal s_cg50FrameRadius = 5.0;
    const QColor s_cg50SelectedFrame(99, 207, 255);

    const char *formatOf(Platform platform)
    {
        if (platform == Platform::NintendoDs)
        {
            return "bmp";
        }
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
    // instead of whatever color happened to sit underneath it.
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

    // Median cut: the box of colors spread widest along one channel is split at
    // its median until there are enough boxes, and each box becomes its average.
    QList<QRgb> fewColors(std::vector<QRgb> colors, int count)
    {
        struct Box
        {
            size_t begin;
            size_t end;
        };

        const auto channel = [](QRgb color, int index)
        { return index == 0 ? qRed(color) : index == 1 ? qGreen(color) : qBlue(color); };
        const auto widest = [&](const Box &box, int *range)
        {
            int best = 0;
            *range = -1;
            for (int index = 0; index < 3; ++index)
            {
                int low = 255, high = 0;
                for (size_t color = box.begin; color < box.end; ++color)
                {
                    low = std::min(low, channel(colors[color], index));
                    high = std::max(high, channel(colors[color], index));
                }
                if (high - low > *range)
                {
                    *range = high - low;
                    best = index;
                }
            }
            return best;
        };

        QList<Box> boxes = {{0, colors.size()}};
        while (boxes.size() < count)
        {
            int chosen = -1, chosenRange = 0, chosenChannel = 0;
            for (int index = 0; index < boxes.size(); ++index)
            {
                int range = 0;
                const int along = widest(boxes[index], &range);
                if (range > chosenRange)
                {
                    chosen = index;
                    chosenRange = range;
                    chosenChannel = along;
                }
            }
            if (chosen < 0)
            {
                break;
            }

            const Box box = boxes[chosen];
            const size_t middle = box.begin + (box.end - box.begin) / 2;
            std::nth_element(colors.begin() + box.begin, colors.begin() + middle, colors.begin() + box.end,
                             [&](QRgb a, QRgb b) { return channel(a, chosenChannel) < channel(b, chosenChannel); });
            boxes[chosen] = {box.begin, middle};
            boxes.append({middle, box.end});
        }

        QList<QRgb> palette;
        for (const Box &box : boxes)
        {
            int red = 0, green = 0, blue = 0;
            for (size_t color = box.begin; color < box.end; ++color)
            {
                red += qRed(colors[color]);
                green += qGreen(colors[color]);
                blue += qBlue(colors[color]);
            }
            const int size = (int)(box.end - box.begin);
            palette << qRgb(red / size, green / size, blue / size);
        }
        return palette;
    }

    int nearestOf(const QList<QRgb> &palette, QRgb color)
    {
        int nearest = 1, nearestDistance = INT_MAX;
        for (int index = 1; index < palette.size(); ++index)
        {
            const int red = qRed(palette[index]) - qRed(color);
            const int green = qGreen(palette[index]) - qGreen(color);
            const int blue = qBlue(palette[index]) - qBlue(color);
            const int distance = red * red + green * green + blue * blue;
            if (distance < nearestDistance)
            {
                nearest = index;
                nearestDistance = distance;
            }
        }
        return nearest;
    }

    // A 16-color palette whose first entry is transparent, which is how the DS
    // menu reads an icon. ndstool takes a BMP of more colors without a warning
    // and garbles it, so the colors are cut here.
    QImage forNintendoDs(const QImage &source, QStringList *notes)
    {
        const QImage resized = resizedTo(squared(source, notes), s_nintendoDsIconSize, notes)
                                   .convertToFormat(QImage::Format_ARGB32);

        std::vector<QRgb> opaque;
        bool partlyTransparent = false;
        for (int y = 0; y < resized.height(); ++y)
        {
            for (int x = 0; x < resized.width(); ++x)
            {
                const QRgb pixel = resized.pixel(x, y);
                partlyTransparent = partlyTransparent || (qAlpha(pixel) > 0 && qAlpha(pixel) < 255);
                if (qAlpha(pixel) >= 128)
                {
                    opaque.push_back(pixel | 0xff000000u);
                }
            }
        }
        if (partlyTransparent)
        {
            *notes << QStringLiteral("Made transparent where less than half opaque and opaque elsewhere, since the "
                                     "DS icon has no partial transparency.");
        }

        QList<QRgb> palette = {qRgba(0, 0, 0, 0)};
        if (!opaque.empty())
        {
            std::vector<QRgb> distinct = opaque;
            std::sort(distinct.begin(), distinct.end());
            distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
            if ((int)distinct.size() > s_nintendoDsOpaqueColors)
            {
                *notes << QStringLiteral("Approximated to %1 colors, which is all the DS icon holds besides "
                                         "transparency.")
                              .arg(s_nintendoDsOpaqueColors);
                palette += fewColors(opaque, s_nintendoDsOpaqueColors);
            }
            else
            {
                palette += QList<QRgb>(distinct.begin(), distinct.end());
            }
        }

        QImage indexed(resized.size(), QImage::Format_Indexed8);
        indexed.setColorTable(palette);
        for (int y = 0; y < resized.height(); ++y)
        {
            for (int x = 0; x < resized.width(); ++x)
            {
                const QRgb pixel = resized.pixel(x, y);
                indexed.setPixel(x, y, qAlpha(pixel) >= 128 ? nearestOf(palette, pixel) : 0);
            }
        }
        return indexed;
    }

    QImage cg50Framed(const QImage &art, const QColor &frame)
    {
        QImage framed(s_cg50IconSize, QImage::Format_RGB32);
        framed.fill(Qt::white);
        QPainter painter(&framed);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(frame);
        painter.drawRoundedRect(QRectF(QPointF(0, 0), QSizeF(s_cg50IconSize)), s_cg50FrameRadius, s_cg50FrameRadius);
        painter.drawImage(s_cg50FrameWidth, s_cg50FrameWidth, art);
        painter.end();
        return framed;
    }

    // The menu draws its icons opaque, on white, so the art is flattened onto
    // white rather than the black the consoles get.
    QImage forCg50(const QImage &source, QStringList *notes)
    {
        const QSize artSize = s_cg50IconSize - QSize(s_cg50FrameWidth * 2, s_cg50FrameWidth * 2);
        QImage art = ImageConversion::resized(ImageConversion::cropped(source, artSize, notes), artSize, notes);
        if (art.hasAlphaChannel())
        {
            *notes << QStringLiteral("Flattened onto white, since the calculator's menu draws icons without "
                                     "transparency.");
            QImage flattened(art.size(), QImage::Format_RGB32);
            flattened.fill(Qt::white);
            QPainter painter(&flattened);
            painter.drawImage(0, 0, art);
            painter.end();
            art = flattened;
        }
        return cg50Framed(art, Qt::white);
    }

    // The selected icon is the unselected one with its frame turned blue.
    QImage cg50Selected(const QImage &unselected)
    {
        const QRect art(s_cg50FrameWidth, s_cg50FrameWidth, s_cg50IconSize.width() - s_cg50FrameWidth * 2,
                        s_cg50IconSize.height() - s_cg50FrameWidth * 2);
        return cg50Framed(unselected.copy(art), s_cg50SelectedFrame);
    }

    QString describeOne(Platform platform, const QImage &image)
    {
        if (platform == Platform::Vita)
        {
            return QStringLiteral("%1 a %2x%2 palette PNG of %3 colors")
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

        if (platform == Platform::NintendoDs)
        {
            return QStringLiteral("%1 a %2x%2 BMP of %3 colors")
                .arg(Icon::path(platform))
                .arg(image.width())
                .arg(image.colorCount());
        }

        if (platform == Platform::Cg50)
        {
            return QStringLiteral("%1 a %2x%3 PNG, with %4 its selected twin")
                .arg(Icon::path(platform))
                .arg(image.width())
                .arg(image.height())
                .arg(s_cg50SelectedPath);
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

    if (platform == Platform::NintendoDs)
    {
        return s_nintendoDsPath;
    }

    if (platform == Platform::Cg50)
    {
        return s_cg50Path;
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

    if (platform == Platform::NintendoDs)
    {
        return forNintendoDs(source, notes);
    }

    if (platform == Platform::Cg50)
    {
        return forCg50(source, notes);
    }

    return forPc(source, notes);
}

bool Icon::write(const QString &projectDirectory, const QMap<Platform, QImage> &icons, QString *error)
{
    QList<ImageConversion::File> files;
    for (auto icon = icons.constBegin(); icon != icons.constEnd(); ++icon)
    {
        files.append({path(icon.key()), icon.value(), formatOf(icon.key())});
        if (icon.key() == Platform::Cg50 && !icon.value().isNull())
        {
            files.append({s_cg50SelectedPath, cg50Selected(icon.value()), formatOf(icon.key())});
        }
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
