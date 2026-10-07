#pragma once

#include "platform.h"

#include <QJsonObject>
#include <QString>

#include <optional>

// The size is only a suggestion until a mode that uses it is picked.
struct DisplaySettings
{
    QString m_mode = QStringLiteral("default");
    int m_width = 640;
    int m_height = 360;
    QString m_filter = QStringLiteral("nearest");

    bool operator==(const DisplaySettings &other) const;
    bool operator!=(const DisplaySettings &other) const;
};

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
    DisplaySettings display() const;
    // Empty when the platform takes the display every platform shares.
    std::optional<DisplaySettings> display(Platform platform) const;

    void setName(const QString &value);
    void setOrganization(const QString &value);
    void setVersion(const QString &value);
    void setWindowTitle(const QString &value);
    void setVitaTitleId(const QString &value);
    void setDisplay(const DisplaySettings &value);
    void setDisplay(Platform platform, const std::optional<DisplaySettings> &value);

private:
    QString m_directory;

    // Kept whole so unknown keys survive a save.
    QJsonObject m_root;
};
