#include "downloads.h"

#include "windows_toolchain.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

namespace
{
    const QString s_unpackedMarker = QStringLiteral(".fried-unpacked");
}

QString Downloads::dependencySourcesDirectory()
{
#ifdef Q_OS_WIN
    const QString base = qEnvironmentVariable("LOCALAPPDATA");
#else
    const QString xdgCache = qEnvironmentVariable("XDG_CACHE_HOME");
    const QString base = xdgCache.isEmpty() ? QDir::home().filePath(QStringLiteral(".cache")) : xdgCache;
#endif
    return QDir(base).filePath(QStringLiteral("fried-engine/deps"));
}

QList<Download> Downloads::installed()
{
    QList<Download> found;

    const QDir sources(dependencySourcesDirectory());
    for (const QFileInfo &release : sources.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
    {
        found << Download{release.fileName(), QStringLiteral("Engine dependency source"), release.absoluteFilePath(),
                          QFileInfo::exists(QDir(release.absoluteFilePath()).filePath(s_unpackedMarker))};
    }

    const QString w64devkit = WindowsToolchain::w64devkitDirectory();
    if (QFileInfo::exists(w64devkit))
    {
        found << Download{QFileInfo(w64devkit).fileName(), QStringLiteral("C++ compiler for Windows"), w64devkit,
                          WindowsToolchain::w64devkitInstalled()};
    }

    return found;
}

qint64 Downloads::sizeOf(const QString &path)
{
    qint64 size = 0;
    QDirIterator files(path, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (files.hasNext())
    {
        size += files.nextFileInfo().size();
    }
    return size;
}
