#include "icon.h"

#include "core_bridge.h"

namespace
{
    std::vector<fried::core::RenderedIcon> listOf(const QList<fried::core::RenderedIcon> &icons)
    {
        return std::vector<fried::core::RenderedIcon>(icons.begin(), icons.end());
    }
}

QString Icon::path(Platform platform)
{
    const std::vector<std::string> paths = fried::core::Icon::paths(platform);
    return paths.empty() ? QString() : toQt(paths.front());
}

fried::core::RenderedIcon Icon::render(Platform platform, const QImage &source, QStringList *notes)
{
    std::vector<std::string> sentences;
    const fried::core::RenderedIcon icon = fried::core::Icon::render(platform, fromQt(source), &sentences);
    for (const std::string &sentence : sentences)
    {
        *notes << toQt(sentence);
    }
    return icon;
}

bool Icon::write(const QString &projectDirectory, const QList<fried::core::RenderedIcon> &icons, QString *error)
{
    std::string reason;
    const bool isWritten = fried::core::Icon::write(fromQt(projectDirectory), listOf(icons), &reason);
    if (!isWritten)
    {
        *error = toQt(reason);
    }
    return isWritten;
}

QString Icon::describe(const QList<fried::core::RenderedIcon> &icons)
{
    return toQt(fried::core::Icon::describe(listOf(icons)));
}
