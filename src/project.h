#pragma once

#include "core_bridge.h"
#include "platform.h"

#include <fried/core/project.h>

#include <QString>

using fried::core::DisplaySettings;

// project.fried as the tool edits it. What the file means is FriedCore's; this takes and gives Qt's strings.
class Project
{
public:
    // True when the directory holds a project file, whether or not it parses.
    static bool exists(const QString &directory);

    bool load(const QString &directory, QString *error);
    bool save(QString *error) const;

    // Each returns what is wrong with the value, or an empty string.
    static QString checkIdentity(const QString &value);
    static QString checkVersion(const QString &value);
    static QString checkVitaTitleId(const QString &value);

    QString directory() const;
    QString filePath() const;

    QString name() const;
    QString organization() const;
    QString version() const;
    QString windowTitle() const;
    QString vitaTitleId() const;
    // A line of packaging text a platform's section may give, empty when it falls back to the game's identity.
    QString packagingText(const QString &section, const QString &key) const;
    DisplaySettings display() const;
    bool vsync() const;

    void setName(const QString &value);
    void setOrganization(const QString &value);
    void setVersion(const QString &value);
    void setWindowTitle(const QString &value);
    void setVitaTitleId(const QString &value);
    void setPackagingText(const QString &section, const QString &key, const QString &value);
    void setDisplay(const DisplaySettings &value);
    void setVsync(bool value);
    // A display of a platform's own is no longer read by the engine, so saving takes any out.
    void dropPlatformDisplays();

private:
    fried::core::Project m_core;
};
