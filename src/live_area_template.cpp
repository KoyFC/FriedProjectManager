#include "live_area_template.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>


namespace
{
    const QString s_centreStyle = QStringLiteral("a1");
    const QString s_rightStyle = QStringLiteral("psmobile");

    using Attributes = QList<std::pair<QString, QString>>;

    // Each line copies how VitaShell lays out its own LiveArea, which is known to
    // read well beside a start button on the right.
    struct LineLayout
    {
        QString frame;
        Attributes text;
        Attributes str;
    };

    const std::array<LineLayout, LiveAreaTemplate::LineCount> s_lineLayouts = {{
        {QStringLiteral("frame2"),
         {{"valign", "bottom"}, {"align", "left"}, {"text-align", "left"}, {"text-valign", "bottom"},
          {"line-space", "3"}, {"ellipsis", "on"}},
         {{"size", "50"}, {"bold", "on"}, {"shadow", "on"}}},
        {QStringLiteral("frame3"),
         {{"valign", "top"}, {"align", "left"}, {"text-align", "left"}, {"text-valign", "top"},
          {"line-space", "2"}, {"ellipsis", "on"}},
         {{"size", "22"}, {"shadow", "on"}}},
        {QStringLiteral("frame4"),
         {{"align", "left"}, {"text-align", "left"}, {"word-wrap", "off"}, {"ellipsis", "on"}},
         {{"size", "18"}, {"shadow", "on"}}},
    }};

    void writeAttributes(QXmlStreamWriter &writer, const Attributes &attributes)
    {
        for (const auto &[name, value] : attributes)
        {
            writer.writeAttribute(name, value);
        }
    }

    // The same document whatever its indentation, attribute order or declaration,
    // so two files compare by what the console reads in them. Empty on a parse error.
    QString canonical(const QByteArray &xml, QString *error = nullptr)
    {
        QXmlStreamReader reader(xml);
        QString result;
        while (!reader.atEnd())
        {
            reader.readNext();
            if (reader.isStartElement())
            {
                QStringList attributes;
                for (const QXmlStreamAttribute &attribute : reader.attributes())
                {
                    attributes << QStringLiteral("%1=%2").arg(attribute.name(), attribute.value());
                }
                attributes.sort();
                result += QStringLiteral("<%1 %2>").arg(reader.name(), attributes.join(QChar(' ')));
            }
            else if (reader.isEndElement())
            {
                result += QStringLiteral("</%1>").arg(reader.name());
            }
            else if (reader.isCharacters() && !reader.isWhitespace())
            {
                result += reader.text().trimmed();
            }
        }

        if (reader.hasError())
        {
            if (error)
            {
                *error = QStringLiteral("It is not valid XML (line %1): %2")
                             .arg(reader.lineNumber())
                             .arg(reader.errorString());
            }
            return QString();
        }
        return result;
    }

    int lineOfFrame(const QString &frame)
    {
        for (int line = 0; line < LiveAreaTemplate::LineCount; ++line)
        {
            if (s_lineLayouts[line].frame == frame)
            {
                return line;
            }
        }
        return -1;
    }

    // Takes whatever the form can show and ignores the rest, which the caller
    // detects by comparing the file with what this would write.
    LiveAreaTemplate parse(const QByteArray &xml, bool *knownStyle)
    {
        LiveAreaTemplate parsed;
        *knownStyle = false;

        QXmlStreamReader reader(xml);
        int line = -1;
        while (!reader.atEnd())
        {
            reader.readNext();
            if (!reader.isStartElement())
            {
                continue;
            }

            if (reader.name() == QStringLiteral("livearea"))
            {
                const QString style = reader.attributes().value(QStringLiteral("style")).toString();
                *knownStyle = style == s_centreStyle || style == s_rightStyle;
                parsed.m_style = style == s_rightStyle ? LiveAreaTemplate::Style::Right
                                                       : LiveAreaTemplate::Style::Centre;
                parsed.m_contentRevision =
                    reader.attributes().value(QStringLiteral("content-rev")).toString();
            }
            else if (reader.name() == QStringLiteral("frame"))
            {
                line = lineOfFrame(reader.attributes().value(QStringLiteral("id")).toString());
            }
            else if (reader.name() == QStringLiteral("str") && line >= 0 && parsed.m_lines[line].m_text.isEmpty())
            {
                LiveAreaTemplate::Text &text = parsed.m_lines[line];
                const QString colour = reader.attributes().value(QStringLiteral("color")).toString();
                if (!colour.isEmpty())
                {
                    text.m_colour = QColor(colour);
                }
                text.m_text = reader.readElementText().trimmed();
            }
        }
        return parsed;
    }
}

bool LiveAreaTemplate::Text::operator==(const Text &other) const
{
    return m_text == other.m_text && m_colour == other.m_colour;
}

bool LiveAreaTemplate::operator==(const LiveAreaTemplate &other) const
{
    return m_style == other.m_style && m_lines == other.m_lines && m_contentRevision == other.m_contentRevision;
}

bool LiveAreaTemplate::operator!=(const LiveAreaTemplate &other) const
{
    return !(*this == other);
}

QString LiveAreaTemplate::path()
{
    return QStringLiteral("sce_sys/livearea/contents/template.xml");
}

QByteArray LiveAreaTemplate::toXml() const
{
    QByteArray xml;
    QXmlStreamWriter writer(&xml);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(-1);

    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("livearea"));
    writer.writeAttribute(QStringLiteral("style"), m_style == Style::Right ? s_rightStyle : s_centreStyle);
    writer.writeAttribute(QStringLiteral("format-ver"), QStringLiteral("01.00"));
    writer.writeAttribute(QStringLiteral("content-rev"), m_contentRevision);

    writer.writeStartElement(QStringLiteral("livearea-background"));
    writer.writeTextElement(QStringLiteral("image"), QStringLiteral("bg.png"));
    writer.writeEndElement();

    writer.writeStartElement(QStringLiteral("gate"));
    writer.writeTextElement(QStringLiteral("startup-image"), QStringLiteral("startup.png"));
    writer.writeEndElement();

    // A centred start button leaves no room for text, so none is written.
    for (int line = 0; line < LineCount && m_style == Style::Right; ++line)
    {
        const Text &text = m_lines[line];
        if (text.m_text.isEmpty())
        {
            continue;
        }

        const LineLayout &layout = s_lineLayouts[line];
        writer.writeStartElement(QStringLiteral("frame"));
        writer.writeAttribute(QStringLiteral("id"), layout.frame);
        writer.writeStartElement(QStringLiteral("liveitem"));
        writer.writeStartElement(QStringLiteral("text"));
        writeAttributes(writer, layout.text);
        writer.writeStartElement(QStringLiteral("str"));
        writer.writeAttribute(QStringLiteral("color"), text.m_colour.name(QColor::HexRgb));
        writeAttributes(writer, layout.str);
        writer.writeCharacters(text.m_text);
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
    return xml;
}

LiveAreaTemplateFile LiveAreaTemplateFile::read(const QString &projectDirectory)
{
    LiveAreaTemplateFile result;

    QFile file(QDir(projectDirectory).filePath(LiveAreaTemplate::path()));
    if (!file.open(QIODevice::ReadOnly))
    {
        return result;
    }
    result.m_exists = true;

    const QByteArray xml = file.readAll();
    const QString existing = canonical(xml, &result.m_problem);
    if (!result.m_problem.isEmpty())
    {
        return result;
    }

    bool knownStyle = false;
    result.m_template = parse(xml, &knownStyle);
    if (!knownStyle || existing != canonical(result.m_template.toXml()))
    {
        result.m_problem = QStringLiteral("It holds more than this form shows, so it was probably edited by hand.");
    }
    return result;
}

bool LiveAreaTemplate::write(const QString &projectDirectory, QString *error) const
{
    const QString filePath = QDir(projectDirectory).filePath(path());
    QSaveFile file(filePath);
    if (!QDir().mkpath(QFileInfo(filePath).path()) || !file.open(QIODevice::WriteOnly))
    {
        *error = QStringLiteral("Could not write %1: %2").arg(QDir::toNativeSeparators(filePath), file.errorString());
        return false;
    }

    file.write(toXml());
    if (!file.commit())
    {
        *error = QStringLiteral("Could not write %1: %2").arg(QDir::toNativeSeparators(filePath), file.errorString());
        return false;
    }
    return true;
}
