#include "asset_check.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QImageReader>

#include <cstdint>

namespace
{
    constexpr int s_maxTextureSide = 1024;
    constexpr qint64 s_dsTextureMemory = 256 * 1024;
    constexpr qint64 s_cg50Budget = 2 * 1024 * 1024;

    struct Wav
    {
        bool isValid = false;
        int format = 0;
        int channels = 0;
        int bits = 0;
    };

    quint32 read32(const QByteArray &bytes, qsizetype at)
    {
        return static_cast<quint8>(bytes[at]) | (static_cast<quint8>(bytes[at + 1]) << 8)
               | (static_cast<quint8>(bytes[at + 2]) << 16) | (static_cast<quint32>(static_cast<quint8>(bytes[at + 3])) << 24);
    }

    quint16 read16(const QByteArray &bytes, qsizetype at)
    {
        return static_cast<quint16>(static_cast<quint8>(bytes[at]) | (static_cast<quint8>(bytes[at + 1]) << 8));
    }

    Wav readWav(const QString &file)
    {
        Wav wav;
        QFile stream(file);
        if (!stream.open(QIODevice::ReadOnly))
        {
            return wav;
        }

        // Only the chunks up to the format are needed, which is far less than the samples.
        const QByteArray bytes = stream.read(4096);
        if (bytes.size() < 12 || !bytes.startsWith("RIFF") || bytes.mid(8, 4) != "WAVE")
        {
            return wav;
        }

        for (qsizetype position = 12; position + 8 <= bytes.size();)
        {
            const qsizetype size = read32(bytes, position + 4);
            if (bytes.mid(position, 4) == "fmt " && size >= 16 && position + 8 + 16 <= bytes.size())
            {
                wav.format = read16(bytes, position + 8);
                wav.channels = read16(bytes, position + 10);
                wav.bits = read16(bytes, position + 22);
                wav.isValid = true;
                break;
            }
            position += 8 + size + (size & 1);
        }
        return wav;
    }

    bool endsWithAny(const QString &path, const QStringList &suffixes)
    {
        for (const QString &suffix : suffixes)
        {
            if (path.endsWith(suffix, Qt::CaseInsensitive))
            {
                return true;
            }
        }
        return false;
    }

    void checkImage(Platform platform, const QString &relative, const QString &file, QList<AssetIssue> &issues)
    {
        const QSize size = QImageReader(file).size();
        if (!size.isValid())
        {
            return;
        }

        const QString console = platform == Platform::NintendoDs ? QStringLiteral("DS") : QStringLiteral("3DS");
        if (size.width() > s_maxTextureSide || size.height() > s_maxTextureSide)
        {
            issues.append({true, relative,
                           QStringLiteral("The %1 cannot hold a texture over %2 pixels a side, and this is %3x%4.")
                               .arg(console)
                               .arg(s_maxTextureSide)
                               .arg(size.width())
                               .arg(size.height())});
        }
        else if (platform == Platform::NintendoDs && static_cast<qint64>(size.width()) * size.height() * 2 > s_dsTextureMemory)
        {
            issues.append({false, relative,
                           QStringLiteral("At %1x%2 this texture alone is over the DS's 256 KB of texture memory.")
                               .arg(size.width())
                               .arg(size.height())});
        }
    }

    void checkAudio(Platform platform, const QString &relative, const QString &file, QList<AssetIssue> &issues)
    {
        if (platform == Platform::Cg50)
        {
            issues.append({false, relative, QStringLiteral("The fx-CG50 has no audio, so this file only takes up space.")});
            return;
        }

        if (platform != Platform::NintendoDs)
        {
            return;
        }

        const Wav wav = readWav(file);
        if (!wav.isValid)
        {
            issues.append({true, relative, QStringLiteral("The DS plays PCM WAV only, and this is not one.")});
        }
        else if (wav.format != 1 || (wav.bits != 8 && wav.bits != 16))
        {
            issues.append({true, relative, QStringLiteral("The DS plays uncompressed 8 or 16 bit WAV, and this is not.")});
        }
        else if (wav.channels != 1)
        {
            issues.append({true, relative, QStringLiteral("The DS plays mono WAV, and this has %1 channels.").arg(wav.channels)});
        }
    }
}

bool AssetCheck::appliesTo(Platform platform)
{
    return platform == Platform::Nintendo3ds || platform == Platform::NintendoDs || platform == Platform::Cg50;
}

QList<AssetIssue> AssetCheck::validate(Platform platform, const QString &projectDirectory)
{
    QList<AssetIssue> issues;
    const QDir assets(QDir(projectDirectory).filePath(QStringLiteral("assets")));

    qint64 total = 0;
    QDirIterator files(assets.path(), QDir::Files, QDirIterator::Subdirectories);
    while (files.hasNext())
    {
        const QString file = files.next();
        const QString relative = assets.relativeFilePath(file);
        total += files.fileInfo().size();

        if (endsWithAny(file, {QStringLiteral(".png"), QStringLiteral(".jpg"), QStringLiteral(".jpeg")}))
        {
            if (platform == Platform::Nintendo3ds || platform == Platform::NintendoDs)
            {
                checkImage(platform, relative, file, issues);
            }
        }
        else if (endsWithAny(file, {QStringLiteral(".wav"), QStringLiteral(".ogg"), QStringLiteral(".mp3")}))
        {
            if (file.endsWith(QStringLiteral(".wav"), Qt::CaseInsensitive) || platform == Platform::Cg50)
            {
                checkAudio(platform, relative, file, issues);
            }
            else if (platform == Platform::NintendoDs)
            {
                issues.append({true, relative, QStringLiteral("The DS plays PCM WAV only, and this is not one.")});
            }
        }
    }

    if (platform == Platform::Cg50 && total > s_cg50Budget)
    {
        issues.append({true, QString(),
                       QStringLiteral("The assets add up to %1 KB, and the fx-CG50 has room for 2048 KB beside the game.")
                           .arg(total / 1024)});
    }
    return issues;
}
