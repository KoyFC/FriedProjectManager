#pragma once

#include <QJsonObject>
#include <QString>

// The whole object is kept so unknown keys survive being written back.
class Project
{
public:
    bool load(const QString &directory, QString *error);

    QString directory() const;
    QString filePath() const;

    QString name() const;
    QString organization() const;
    QString version() const;
    QString windowTitle() const;
    QString vitaTitleId() const;

private:
    QString m_directory;
    QJsonObject m_root;
};
