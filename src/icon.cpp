#include "icon.h"

#include "image_conversion.h"

#include <QColor>
#include <QPainter>

namespace
{
    const QString s_pcPath = QStringLiteral("assets/icon.png");
    const QString s_vitaPath = QStringLiteral("sce_sys/icon0.png");
    const QString s_switchPath = QStringLiteral("switch/icon.jpg");
    const QString s_nintendo3dsPath = QStringLiteral("3ds/icon.png");
    const QString s_cg50Path = QStringLiteral("cg50/icon-uns.png");
    const QString s_cg50SelectedPath = QStringLiteral("cg50/icon-sel.png");

    constexpr int s_vitaIconSize = 128;
    constexpr int s_switchIconSize = 256;
    constexpr int s_nintendo3dsIconSize = 48;
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
