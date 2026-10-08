#pragma once

#include <QColor>
#include <QString>

#include <array>

// What the form shows of sce_sys/livearea/contents/template.xml: where the start
// button sits and, when it leaves room, three lines of text beside it.
struct LiveAreaTemplate
{
    enum class Style
    {
        Centre,
        Right
    };

    enum Line
    {
        Title,
        Subtitle,
        Footer,
        LineCount
    };

    struct Text
    {
        QString m_text;
        QColor m_colour = Qt::white;

        bool operator==(const Text &other) const;
    };

    Style m_style = Style::Centre;
    std::array<Text, LineCount> m_lines;
    QString m_contentRevision = QStringLiteral("1");

    bool operator==(const LiveAreaTemplate &other) const;
    bool operator!=(const LiveAreaTemplate &other) const;

    static QString path();

    QByteArray toXml() const;
    bool write(const QString &projectDirectory, QString *error) const;
};

// What a project's template.xml holds, as much of it as the form can show.
struct LiveAreaTemplateFile
{
    bool m_exists = false;

    // Empty when the file holds exactly what the form shows.
    QString m_problem;
    LiveAreaTemplate m_template;

    static LiveAreaTemplateFile read(const QString &projectDirectory);
};
