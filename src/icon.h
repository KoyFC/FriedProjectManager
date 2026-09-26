#pragma once

#include <QString>
#include <QStringList>

// A project keeps two icons, because the console's installer reads a palette PNG of
// one fixed size while a window icon is truecolor of any size.
class Icon
{
public:
    static QString pcPath();
    static QString vitaPath();

    // Writes both from one image. The report says what the source had to become.
    static bool write(const QString &projectDirectory, const QString &sourceImage, QString *report, QString *error);

    // Every format this build of Qt can read, as file dialog patterns.
    static QStringList readablePatterns();
};
