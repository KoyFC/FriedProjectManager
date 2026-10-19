#include "engine_version.h"

#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QProcess>

#include <algorithm>

namespace
{
    const QString s_headsPrefix = QStringLiteral("refs/heads/");
    const QString s_tagsPrefix = QStringLiteral("refs/tags/");
    const QString s_peeledSuffix = QStringLiteral("^{}");
    const QString s_defaultBranch = QStringLiteral("main");

    QString gitOutput(const QString &workingDirectory, const QStringList &arguments)
    {
        QProcess git;
        git.setWorkingDirectory(workingDirectory);
        git.start(QStringLiteral("git"), arguments);
        if (!git.waitForFinished() || git.exitStatus() != QProcess::NormalExit || git.exitCode() != 0)
        {
            return QString();
        }

        return QString::fromUtf8(git.readAllStandardOutput()).trimmed();
    }
}

QString EngineVersion::reference() const
{
    return (kind == Kind::Branch ? s_headsPrefix : s_tagsPrefix) + name;
}

QString EngineSubmodule::directory(const QString &projectDirectory)
{
    return QDir(projectDirectory).filePath(QStringLiteral("engine"));
}

// A submodule's checkout holds a .git file pointing into the project's repository.
bool EngineSubmodule::exists(const QString &projectDirectory)
{
    return QFileInfo::exists(QDir(directory(projectDirectory)).filePath(QStringLiteral(".git")));
}

QString EngineSubmodule::currentCommit(const QString &projectDirectory)
{
    if (!exists(projectDirectory))
    {
        return QString();
    }

    return gitOutput(directory(projectDirectory), {QStringLiteral("rev-parse"), QStringLiteral("HEAD")});
}

QString EngineSubmodule::currentDescription(const QString &projectDirectory)
{
    if (!exists(projectDirectory))
    {
        return QString();
    }

    return gitOutput(directory(projectDirectory),
                     {QStringLiteral("log"), QStringLiteral("-1"), QStringLiteral("--format=%h, %cs: %s")});
}

QStringList EngineSubmodule::listRemoteVersionsCommand()
{
    return {QStringLiteral("git"), QStringLiteral("ls-remote"), QStringLiteral("--sort=-v:refname"),
            QStringLiteral("--heads"), QStringLiteral("--tags"), QStringLiteral("origin")};
}

QList<EngineVersion> EngineSubmodule::parseRemoteVersions(const QString &listing)
{
    QList<EngineVersion> branches;
    QList<EngineVersion> tags;

    // An annotated tag is listed twice, and only its peeled line names the commit.
    QMap<QString, QString> peeledTagCommits;

    for (const QString &line : listing.split(QChar('\n'), Qt::SkipEmptyParts))
    {
        const QStringList fields = line.split(QChar('\t'));
        if (fields.size() != 2)
        {
            continue;
        }

        const QString commit = fields[0].trimmed();
        QString reference = fields[1].trimmed();

        if (reference.startsWith(s_headsPrefix))
        {
            branches << EngineVersion{EngineVersion::Kind::Branch, reference.mid(s_headsPrefix.size()), commit};
        }
        else if (reference.startsWith(s_tagsPrefix) && reference.endsWith(s_peeledSuffix))
        {
            reference.chop(s_peeledSuffix.size());
            peeledTagCommits.insert(reference.mid(s_tagsPrefix.size()), commit);
        }
        else if (reference.startsWith(s_tagsPrefix))
        {
            tags << EngineVersion{EngineVersion::Kind::Tag, reference.mid(s_tagsPrefix.size()), commit};
        }
    }

    for (EngineVersion &tag : tags)
    {
        tag.commit = peeledTagCommits.value(tag.name, tag.commit);
    }

    std::stable_sort(branches.begin(), branches.end(), [](const EngineVersion &left, const EngineVersion &right) {
        return left.name == s_defaultBranch && right.name != s_defaultBranch;
    });

    return branches + tags;
}

// A submodule sits on a commit rather than a branch, so the fetched one is checked out detached.
QList<QStringList> EngineSubmodule::switchCommands(const EngineVersion &version)
{
    return {
        {QStringLiteral("git"), QStringLiteral("fetch"), QStringLiteral("origin"), version.reference()},
        {QStringLiteral("git"), QStringLiteral("checkout"), QStringLiteral("--detach"), QStringLiteral("FETCH_HEAD")},
    };
}
