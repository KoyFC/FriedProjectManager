#pragma once

#include <QString>

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
