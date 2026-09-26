#include "project_template.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>

namespace
{
    const QString s_templateRoot = QStringLiteral(":/templates/project");
    const QString s_defaultVersion = QStringLiteral("01.00");
    const QString s_engineUrl = QStringLiteral("https://github.com/KoyFC/FriedEngine.git");
    const QString s_engineDirectory = QStringLiteral("engine");

    const QStringList s_substituted = {
        QStringLiteral(".vscode/c_cpp_properties.json"),
        QStringLiteral(".vscode/launch.json"),
        QStringLiteral(".vscode/settings.json"),
        QStringLiteral(".vscode/tasks.json"),
        QStringLiteral("CMakeLists.txt"),
        QStringLiteral("project.fried"),
    };

    QString targetName(const QString &name)
    {
        static const QRegularExpression separators(QStringLiteral("[^a-z0-9]+"));
        const QString target = name.toLower().split(separators, Qt::SkipEmptyParts).join(QChar('_'));
        return target.isEmpty() ? QStringLiteral("game") : target;
    }

    // A valid placeholder, since a Vita package will not build without one.
    QString titleId(const QString &name)
    {
        QString letters;
        for (const QChar character : name)
        {
            if (character.isLetter() && character.unicode() < 128)
            {
                letters += character.toUpper();
            }

            if (letters.size() == 4)
            {
                break;
            }
        }

        return letters.leftJustified(4, QChar('X')) + QStringLiteral("00001");
    }

    QByteArray substituted(const QByteArray &contents, const QHash<QString, QString> &values)
    {
        QString text = QString::fromUtf8(contents);
        for (auto value = values.constBegin(); value != values.constEnd(); ++value)
        {
            text.replace(QStringLiteral("@%1@").arg(value.key()), value.value());
        }

        return text.toUtf8();
    }

    bool copyFile(const QString &relativePath, const QDir &target, const QHash<QString, QString> &values, QString *error)
    {
        QFile source(QStringLiteral("%1/%2").arg(s_templateRoot, relativePath));
        if (!source.open(QIODevice::ReadOnly))
        {
            *error = QStringLiteral("Could not read the template for %1.").arg(relativePath);
            return false;
        }

        QByteArray contents = source.readAll();
        if (s_substituted.contains(relativePath))
        {
            contents = substituted(contents, values);
        }

        const QString destination = target.filePath(relativePath);
        if (!target.mkpath(QFileInfo(relativePath).path()))
        {
            *error = QStringLiteral("Could not create %1.").arg(QDir::toNativeSeparators(QFileInfo(destination).path()));
            return false;
        }

        QFile output(destination);
        if (!output.open(QIODevice::WriteOnly) || output.write(contents) != contents.size())
        {
            *error = QStringLiteral("Could not write %1: %2")
                         .arg(QDir::toNativeSeparators(destination), output.errorString());
            return false;
        }

        output.close();

        // A resource carries no file mode, and the editor tasks have to be able to run these.
        const QFileDevice::Permissions executable = QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeOther;
        if (relativePath.endsWith(QStringLiteral(".sh")) && !output.setPermissions(output.permissions() | executable))
        {
            *error = QStringLiteral("Could not make %1 executable.").arg(QDir::toNativeSeparators(destination));
            return false;
        }

        return true;
    }
}

bool ProjectTemplate::write(const QString &directory, const QString &name, const QString &organization, QString *error)
{
    QDir target(directory);
    if (target.exists() && !target.isEmpty())
    {
        *error = QStringLiteral("%1 already exists and is not empty.").arg(QDir::toNativeSeparators(directory));
        return false;
    }

    if (!target.mkpath(QStringLiteral(".")))
    {
        *error = QStringLiteral("Could not create %1.").arg(QDir::toNativeSeparators(directory));
        return false;
    }

    // Project::checkIdentity rejects the characters that would need JSON escaping.
    const QHash<QString, QString> values = {
        {QStringLiteral("NAME"), name},
        {QStringLiteral("ORGANIZATION"), organization},
        {QStringLiteral("VERSION"), s_defaultVersion},
        {QStringLiteral("TITLE_ID"), titleId(name)},
        {QStringLiteral("TARGET"), targetName(name)},
    };

    QDirIterator templateFiles(s_templateRoot, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (templateFiles.hasNext())
    {
        const QString relativePath = templateFiles.next().mid(s_templateRoot.size() + 1);
        if (!copyFile(relativePath, target, values, error))
        {
            return false;
        }
    }

    return true;
}

QList<QStringList> ProjectTemplate::repositoryCommands()
{
    return {
        {QStringLiteral("git"), QStringLiteral("init"), QStringLiteral("-b"), QStringLiteral("main")},
        {QStringLiteral("git"), QStringLiteral("submodule"), QStringLiteral("add"), s_engineUrl, s_engineDirectory},
        {QStringLiteral("git"), QStringLiteral("add"), QStringLiteral("-A")},
    };
}
