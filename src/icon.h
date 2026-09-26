#pragma once

#include <QImage>
#include <QString>
#include <QStringList>

// A project keeps two icons, because the console's installer reads a palette PNG of
// one fixed size while a window icon is truecolor of any size.
class Icon
{
public:
    static QString pcPath();
    static QString vitaPath();

    // Null when the file cannot be read as an image, with the reason in error.
    static QImage read(const QString &sourceImage, QString *error);

    // What each file would hold, saying in notes whatever the source had to become.
    static QImage forPc(const QImage &source, QStringList *notes);
    static QImage forVita(const QImage &source, QStringList *notes);

    // A null image leaves that file as it is.
    static bool write(const QString &projectDirectory, const QImage &pc, const QImage &vita, QString *error);
    static QString describe(const QImage &pc, const QImage &vita);

    // Every format this build of Qt can read, as file dialog patterns.
    static QStringList readablePatterns();
};
