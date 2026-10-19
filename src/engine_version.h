#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// A branch or tag of the engine repository the project's submodule can be moved to.
struct EngineVersion
{
    enum class Kind
    {
        Branch,
        Tag
    };

    Kind kind = Kind::Branch;
    QString name;
    QString commit;

    QString reference() const;
};

namespace EngineSubmodule
{
    QString directory(const QString &projectDirectory);
    bool exists(const QString &projectDirectory);

    // Empty when there is no checkout to ask.
    QString currentCommit(const QString &projectDirectory);
    QString currentDescription(const QString &projectDirectory);

    QStringList listRemoteVersionsCommand();
    QList<EngineVersion> parseRemoteVersions(const QString &listing);

    // Run inside the submodule.
    QList<QStringList> switchCommands(const EngineVersion &version);
}
