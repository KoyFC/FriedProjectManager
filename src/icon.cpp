#include "icon.h"

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QSaveFile>

namespace
{
    const QString s_pcPath = QStringLiteral("assets/icon.png");
    const QString s_vitaPath = QStringLiteral("sce_sys/icon0.png");

    // What the console's installer accepts, and nothing else.
    constexpr int s_vitaSize = 128;

    // No desktop draws a window icon larger than this.
    constexpr int s_pcLimit = 512;

    // An icon is square wherever it is shown, so an oblong source keeps its middle.
    QImage squared(const QImage &image)
    {
        const int side = qMin(image.width(), image.height());
        return image.copy((image.width() - side) / 2, (image.height() - side) / 2, side, side);
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

bool Icon::write(const QString &projectDirectory, const QString &sourceImage, QString *report, QString *error)
{
    QImageReader reader(sourceImage);

    // A photograph straight from a camera is stored rotated.
    reader.setAutoTransform(true);

    const QImage source = reader.read();
    if (source.isNull())
    {
        *error = QStringLiteral("%1 could not be read as an image: %2")
                     .arg(QFileInfo(sourceImage).fileName(), reader.errorString());
        return false;
    }

    QStringList notes;

    const QImage square = squared(source);
    if (square.size() != source.size())
    {
        notes << QStringLiteral("The source is %1x%2, so its centre %3x%3 square was used.")
                     .arg(source.width())
                     .arg(source.height())
                     .arg(square.width());
    }

    QImage pc = square;
    if (pc.width() > s_pcLimit)
    {
        pc = pc.scaled(s_pcLimit, s_pcLimit, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        notes << QStringLiteral("The window icon was reduced to %1x%1.").arg(s_pcLimit);
    }
    pc = pc.convertToFormat(pc.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

    // Qt keeps every colour of an image that has at most 256 of them, and approximates the rest.
    const QImage vita = square.scaled(s_vitaSize, s_vitaSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                            .convertToFormat(QImage::Format_Indexed8, Qt::AutoColor | Qt::ThresholdDither);
    if (square.width() < s_vitaSize)
    {
        notes << QStringLiteral("The source is only %1x%1, so the Vita icon was enlarged to %2x%2 and will look soft.")
                     .arg(square.width())
                     .arg(s_vitaSize);
    }

    const QDir directory(projectDirectory);
    QSaveFile pcFile(directory.filePath(s_pcPath));
    QSaveFile vitaFile(directory.filePath(s_vitaPath));

    // Both are staged before either is committed, so a failure replaces neither.
    if (!stage(pcFile, pc, error) || !stage(vitaFile, vita, error))
    {
        return false;
    }
    if (!commit(pcFile, error) || !commit(vitaFile, error))
    {
        return false;
    }

    *report = QStringLiteral("%1 is now %2x%2 truecolor, and %3 a %4x%4 palette PNG of %5 colours.")
                  .arg(s_pcPath)
                  .arg(pc.width())
                  .arg(s_vitaPath)
                  .arg(s_vitaSize)
                  .arg(vita.colorCount());
    if (!notes.isEmpty())
    {
        *report += QStringLiteral("\n\n") + notes.join(QChar('\n'));
    }
    return true;
}
