#include "live_area.h"

#include "image_conversion.h"

QString LiveArea::path(Image image)
{
    return image == Image::Background ? QStringLiteral("sce_sys/livearea/contents/bg.png")
                                      : QStringLiteral("sce_sys/livearea/contents/startup.png");
}

QSize LiveArea::size(Image image)
{
    return image == Image::Background ? QSize(840, 500) : QSize(280, 158);
}

QString LiveArea::title(Image image)
{
    return image == Image::Background ? QStringLiteral("Background") : QStringLiteral("Start button");
}

QString LiveArea::description(Image image)
{
    return image == Image::Background ? QStringLiteral("Fills the game's page behind everything else.")
                                      : QStringLiteral("The button that starts the game.");
}

QImage LiveArea::render(Image image, const QImage &source, QStringList *notes)
{
    const QImage shaped = ImageConversion::cropped(source, size(image), notes);
    return ImageConversion::palette(ImageConversion::resized(shaped, size(image), notes), notes);
}
