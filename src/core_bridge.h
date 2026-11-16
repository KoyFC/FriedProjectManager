#pragma once

#include <fried/core/image.h>

#include <QImage>
#include <QString>

#include <cstring>
#include <string>

// FriedCore, which the Fried Editor shares, has no Qt in it: its text is UTF-8 in a std::string.
inline QString toQt(const std::string &text)
{
    return QString::fromStdString(text);
}

inline std::string fromQt(const QString &text)
{
    return text.toStdString();
}

// FriedCore's images are plain RGBA, not premultiplied, which is Qt's Format_RGBA8888.
inline fried::core::Image fromQt(const QImage &source)
{
    fried::core::Image image;
    if (source.isNull())
    {
        return image;
    }

    const QImage rgba = source.convertToFormat(QImage::Format_RGBA8888);
    image.m_width = rgba.width();
    image.m_height = rgba.height();
    image.m_hasAlpha = source.hasAlphaChannel();
    image.m_rgba.resize(static_cast<std::size_t>(image.m_width) * image.m_height * 4);
    for (int y = 0; y < image.m_height; ++y)
    {
        std::memcpy(image.m_rgba.data() + static_cast<std::size_t>(y) * image.m_width * 4, rgba.constScanLine(y),
                    static_cast<std::size_t>(image.m_width) * 4);
    }
    return image;
}

inline QImage toQt(const fried::core::Image &source)
{
    if (source.isNull())
    {
        return QImage();
    }

    QImage image(source.m_width, source.m_height, QImage::Format_RGBA8888);
    for (int y = 0; y < source.m_height; ++y)
    {
        std::memcpy(image.scanLine(y), source.m_rgba.data() + static_cast<std::size_t>(y) * source.m_width * 4,
                    static_cast<std::size_t>(source.m_width) * 4);
    }
    return image;
}
