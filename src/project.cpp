#include "project.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>

namespace
{
    const QString s_fileName = QStringLiteral("project.fried");

    QString nested(const QJsonObject &root, const QString &key, const QString &subKey)
    {
        return root.value(key).toObject().value(subKey).toString();
    }

    // The engine reads an absent key as undeclared, so an empty value removes it.
    void assign(QJsonObject &object, const QString &key, const QString &value)
    {
        if (value.isEmpty())
        {
            object.remove(key);
        }
        else
        {
            object.insert(key, value);
        }
    }

    void assign(QJsonObject &object, const QString &key, const QJsonObject &value)
    {
        if (value.isEmpty())
        {
            object.remove(key);
        }
        else
        {
            object.insert(key, value);
        }
    }

    void assignNested(QJsonObject &root, const QString &key, const QString &subKey, const QString &value)
    {
        QJsonObject child = root.value(key).toObject();
        assign(child, subKey, value);
        assign(root, key, child);
    }

    // Qt indents with four spaces; these files use tabs.
    QByteArray tabIndented(const QByteArray &json)
    {
        QByteArrayList lines = json.split('\n');
        for (QByteArray &line : lines)
        {
            qsizetype spaces = 0;
            while (spaces < line.size() && line.at(spaces) == ' ')
            {
                ++spaces;
            }
            line = QByteArray(spaces / 4, '\t') + line.sliced(spaces);
        }
        return lines.join('\n');
    }
}

// Fields are checked on save, not here.
bool Project::exists(const QString &directory)
{
    return QFileInfo::exists(QDir(directory).filePath(s_fileName));
}

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

bool Project::save(QString *error) const
{
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        *error = QStringLiteral("Could not write %1: %2")
                     .arg(QDir::toNativeSeparators(filePath()), file.errorString());
        return false;
    }

    file.write(tabIndented(QJsonDocument(m_root).toJson(QJsonDocument::Indented)));

    if (!file.commit())
    {
        *error = QStringLiteral("Could not write %1: %2")
                     .arg(QDir::toNativeSeparators(filePath()), file.errorString());
        return false;
    }

    return true;
}

// Both become directory names through SDL_GetPrefPath.
QString Project::checkIdentity(const QString &value)
{
    if (value.isEmpty())
    {
        return QStringLiteral("must not be empty.");
    }

    static const QRegularExpression forbidden(QStringLiteral(R"([\\/:*?"<>|])"));
    if (value.contains(forbidden))
    {
        return QStringLiteral(R"(may not contain any of \ / : * ? " < > |, since it becomes a directory name.)");
    }

    return QString();
}

QString Project::checkVersion(const QString &value)
{
    static const QRegularExpression shape(QStringLiteral(R"(\A\d{2}\.\d{2}\z)"));
    if (!value.isEmpty() && !shape.match(value).hasMatch())
    {
        return QStringLiteral("must be two digits, a dot and two digits, which is what a Vita package requires (01.00).");
    }

    return QString();
}

QString Project::checkVitaTitleId(const QString &value)
{
    static const QRegularExpression shape(QStringLiteral(R"(\A[A-Z]{4}\d{5}\z)"));
    if (!value.isEmpty() && !shape.match(value).hasMatch())
    {
        return QStringLiteral("must be four capital letters followed by five digits (FRIE00001).");
    }

    return QString();
}

void Project::setName(const QString &value)
{
    assign(m_root, QStringLiteral("name"), value);
}

void Project::setOrganization(const QString &value)
{
    assign(m_root, QStringLiteral("organization"), value);
}

void Project::setVersion(const QString &value)
{
    assign(m_root, QStringLiteral("version"), value);
}

void Project::setWindowTitle(const QString &value)
{
    assignNested(m_root, QStringLiteral("window"), QStringLiteral("title"), value);
}

void Project::setVitaTitleId(const QString &value)
{
    assignNested(m_root, QStringLiteral("vita"), QStringLiteral("titleId"), value);
}
