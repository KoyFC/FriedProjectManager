#include "project.h"

bool Project::exists(const QString &directory)
{
    return fried::core::Project::exists(fromQt(directory));
}

bool Project::load(const QString &directory, QString *error)
{
    std::string reason;
    const bool isLoaded = m_core.load(fromQt(directory), &reason);
    if (!isLoaded)
    {
        *error = toQt(reason);
    }
    return isLoaded;
}

bool Project::save(QString *error) const
{
    std::string reason;
    const bool isSaved = m_core.save(&reason);
    if (!isSaved)
    {
        *error = toQt(reason);
    }
    return isSaved;
}

QString Project::checkIdentity(const QString &value)
{
    return toQt(fried::core::Project::checkIdentity(fromQt(value)));
}

QString Project::checkVersion(const QString &value)
{
    return toQt(fried::core::Project::checkVersion(fromQt(value)));
}

QString Project::checkVitaTitleId(const QString &value)
{
    return toQt(fried::core::Project::checkVitaTitleId(fromQt(value)));
}

QString Project::directory() const
{
    return toQt(m_core.directory());
}

QString Project::filePath() const
{
    return toQt(m_core.filePath());
}

QString Project::name() const
{
    return toQt(m_core.name());
}

QString Project::organization() const
{
    return toQt(m_core.organization());
}

QString Project::version() const
{
    return toQt(m_core.version());
}

QString Project::windowTitle() const
{
    return toQt(m_core.windowTitle());
}

QString Project::vitaTitleId() const
{
    return toQt(m_core.vitaTitleId());
}

QString Project::packagingText(const QString &section, const QString &key) const
{
    return toQt(m_core.packagingText(fromQt(section), fromQt(key)));
}

DisplaySettings Project::display() const
{
    return m_core.display();
}

void Project::setName(const QString &value)
{
    m_core.setName(fromQt(value));
}

void Project::setOrganization(const QString &value)
{
    m_core.setOrganization(fromQt(value));
}

void Project::setVersion(const QString &value)
{
    m_core.setVersion(fromQt(value));
}

void Project::setWindowTitle(const QString &value)
{
    m_core.setWindowTitle(fromQt(value));
}

void Project::setVitaTitleId(const QString &value)
{
    m_core.setVitaTitleId(fromQt(value));
}

void Project::setPackagingText(const QString &section, const QString &key, const QString &value)
{
    m_core.setPackagingText(fromQt(section), fromQt(key), fromQt(value));
}

bool Project::vsync() const
{
    return m_core.vsync();
}

int Project::defaultTargetFramerate() const
{
    return m_core.defaultTargetFramerate();
}

void Project::setDefaultTargetFramerate(int value)
{
    m_core.setDefaultTargetFramerate(value);
}

int Project::physicsRate() const
{
    return m_core.physicsRate();
}

void Project::setPhysicsRate(int value)
{
    m_core.setPhysicsRate(value);
}

bool Project::declares(Platform platform) const
{
    return m_core.declares(platform);
}

void Project::setDeclaredPlatforms(const std::vector<Platform> &platforms)
{
    m_core.setDeclaredPlatforms(platforms);
}

void Project::setVsync(bool value)
{
    m_core.setVsync(value);
}

void Project::setDisplay(const DisplaySettings &value)
{
    m_core.setDisplay(value);
}

void Project::dropPlatformDisplays()
{
    m_core.dropPlatformDisplays();
}
