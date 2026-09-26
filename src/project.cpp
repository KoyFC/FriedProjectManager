#include "project.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

namespace
{
    const QString s_fileName = QStringLiteral("project.fried");

    QString nested(const QJsonObject &root, const QString &key, const QString &subKey)
    {
        return root.value(key).toObject().value(subKey).toString();
    }
}

// No field is required: a manager that refused a file missing one could not be
// used to add it.
bool Project::load(const QString &directory, QString *error)
{
    const QString path = QDir(directory).filePath(s_fileName);

    QFile file(path);
    if (!file.exists())
    {
        *error = QStringLiteral("%1 contains no %2, so it is not a Fried project.")
                     .arg(QDir::toNativeSeparators(directory), s_fileName);
        return false;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        *error = QStringLiteral("Could not read %1: %2")
                     .arg(QDir::toNativeSeparators(path), file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        *error = QStringLiteral("%1 is not valid JSON: %2 at offset %3")
                     .arg(s_fileName, parseError.errorString())
                     .arg(parseError.offset);
        return false;
    }

    if (!document.isObject())
    {
        *error = QStringLiteral("%1 must hold a JSON object.").arg(s_fileName);
        return false;
    }

    m_directory = directory;
    m_root = document.object();
    return true;
}

QString Project::directory() const
{
    return m_directory;
}

QString Project::filePath() const
{
    return QDir(m_directory).filePath(s_fileName);
}

QString Project::name() const
{
    return m_root.value(QStringLiteral("name")).toString();
}

QString Project::organization() const
{
    return m_root.value(QStringLiteral("organization")).toString();
}

QString Project::version() const
{
    return m_root.value(QStringLiteral("version")).toString();
}

QString Project::windowTitle() const
{
    return nested(m_root, QStringLiteral("window"), QStringLiteral("title"));
}

QString Project::vitaTitleId() const
{
    return nested(m_root, QStringLiteral("vita"), QStringLiteral("titleId"));
}
