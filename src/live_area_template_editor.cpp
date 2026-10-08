#include "live_area_template_editor.h"

#include "help_label.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

namespace
{
    constexpr int s_swatchSize = 16;

    struct LineLabels
    {
        QString label;
        QString placeholder;
    };

    const std::array<LineLabels, LiveAreaTemplate::LineCount> s_lineLabels = {{
        {QStringLiteral("Title"), QStringLiteral("Large, such as the game's name")},
        {QStringLiteral("Subtitle"), QStringLiteral("Below the title, such as who made it")},
        {QStringLiteral("Footer"), QStringLiteral("Small, at the bottom, such as the version")},
    }};
}

LiveAreaTemplateEditor::LiveAreaTemplateEditor(QWidget *parent)
    : QWidget(parent)
{
    QLabel *heading = new QLabel(QStringLiteral("Layout"), this);
    QFont bold = heading->font();
    bold.setBold(true);
    heading->setFont(bold);

    m_centre = new QRadioButton(QStringLiteral("Start button in the centre"), this);
    m_right = new QRadioButton(QStringLiteral("Start button on the right, with text beside it"), this);

    QButtonGroup *styles = new QButtonGroup(this);
    styles->addButton(m_centre);
    styles->addButton(m_right);
    connect(styles, &QButtonGroup::buttonToggled, this, [this](QAbstractButton *, bool checked) {
        if (checked)
        {
            formEdited();
        }
    });

    m_lines = new QWidget(this);
    QFormLayout *lines = new QFormLayout(m_lines);
    lines->setContentsMargins(0, 0, 0, 0);
    for (int line = 0; line < LiveAreaTemplate::LineCount; ++line)
    {
        m_texts[line] = new QLineEdit(m_lines);
        m_texts[line]->setPlaceholderText(s_lineLabels[line].placeholder);
        connect(m_texts[line], &QLineEdit::textChanged, this, &LiveAreaTemplateEditor::formEdited);

        m_colourButtons[line] = new QPushButton(QStringLiteral("Colour..."), m_lines);
        connect(m_colourButtons[line], &QPushButton::clicked, this, [this, line] { chooseColour(line); });

        QHBoxLayout *row = new QHBoxLayout;
        row->addWidget(m_texts[line], 1);
        row->addWidget(m_colourButtons[line]);
        lines->addRow(s_lineLabels[line].label, row);
    }

    m_status = helpLabel(QString(), this);

    m_replace = new QPushButton(QStringLiteral("Replace with the Form"), this);
    m_replace->setToolTip(QStringLiteral("Write what the form shows on Save, losing whatever else the file holds."));
    connect(m_replace, &QPushButton::clicked, this, &LiveAreaTemplateEditor::replaceHandEdited);

    QHBoxLayout *replace = new QHBoxLayout;
    replace->addWidget(m_replace);
    replace->addStretch();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(heading);
    layout->addWidget(helpLabel(QStringLiteral("Where the start button sits on the game's page, written to %1. "
                                               "Text is shown in white unless given a colour.")
                                    .arg(LiveAreaTemplate::path()),
                                this));
    layout->addWidget(m_centre);
    layout->addWidget(m_right);
    layout->addWidget(m_lines);
    layout->addWidget(m_status);
    layout->addLayout(replace);
}

void LiveAreaTemplateEditor::setProjectDirectory(const QString &directory)
{
    m_directory = directory;
    m_isReplacing = false;

    const LiveAreaTemplateFile file = LiveAreaTemplateFile::read(directory);
    m_problem = file.m_exists ? file.m_problem : QString();
    m_saved = file.m_exists && file.m_problem.isEmpty() ? std::optional<LiveAreaTemplate>(file.m_template)
                                                         : std::nullopt;
    m_contentRevision = file.m_exists ? file.m_template.m_contentRevision : LiveAreaTemplate().m_contentRevision;
    if (m_contentRevision.isEmpty())
    {
        m_contentRevision = LiveAreaTemplate().m_contentRevision;
    }

    showTemplate(file.m_template);
    emit changed();
}

bool LiveAreaTemplateEditor::hasPending() const
{
    if (!m_problem.isEmpty() && !m_isReplacing)
    {
        return false;
    }

    return !m_saved || templateInForm() != *m_saved;
}

bool LiveAreaTemplateEditor::writePending(QString *error)
{
    if (!hasPending())
    {
        return true;
    }

    const LiveAreaTemplate written = templateInForm();
    if (!written.write(m_directory, error))
    {
        return false;
    }

    m_saved = written;
    m_problem.clear();
    m_isReplacing = false;
    showState();
    return true;
}

void LiveAreaTemplateEditor::showTemplate(const LiveAreaTemplate &shown)
{
    m_isShowing = true;
    (shown.m_style == LiveAreaTemplate::Style::Right ? m_right : m_centre)->setChecked(true);
    for (int line = 0; line < LiveAreaTemplate::LineCount; ++line)
    {
        m_texts[line]->setText(shown.m_lines[line].m_text);
        m_colours[line] = shown.m_lines[line].m_colour.isValid() ? shown.m_lines[line].m_colour : QColor(Qt::white);
        showColour(line);
    }
    m_isShowing = false;

    showState();
}

LiveAreaTemplate LiveAreaTemplateEditor::templateInForm() const
{
    LiveAreaTemplate inForm;
    inForm.m_style = m_right->isChecked() ? LiveAreaTemplate::Style::Right : LiveAreaTemplate::Style::Centre;
    inForm.m_contentRevision = m_contentRevision;

    // Text a centred layout cannot show stays in the fields, but is not part of the file.
    for (int line = 0; line < LiveAreaTemplate::LineCount && inForm.m_style == LiveAreaTemplate::Style::Right; ++line)
    {
        inForm.m_lines[line] = {m_texts[line]->text().trimmed(), m_colours[line]};
    }
    return inForm;
}

void LiveAreaTemplateEditor::formEdited()
{
    if (m_isShowing)
    {
        return;
    }

    showState();
    emit changed();
}

void LiveAreaTemplateEditor::replaceHandEdited()
{
    m_isReplacing = true;
    showState();
    emit changed();
}

void LiveAreaTemplateEditor::chooseColour(int line)
{
    const QColor chosen = QColorDialog::getColor(m_colours[line], this,
                                                 QStringLiteral("%1 Colour").arg(s_lineLabels[line].label));
    if (!chosen.isValid() || chosen == m_colours[line])
    {
        return;
    }

    m_colours[line] = chosen;
    showColour(line);
    formEdited();
}

void LiveAreaTemplateEditor::showColour(int line)
{
    QPixmap swatch(s_swatchSize, s_swatchSize);
    swatch.fill(m_colours[line]);
    m_colourButtons[line]->setIcon(QIcon(swatch));
    m_colourButtons[line]->setToolTip(m_colours[line].name());
}

void LiveAreaTemplateEditor::showState()
{
    const bool isHandEdited = !m_problem.isEmpty() && !m_isReplacing;
    const bool hasRoomForText = m_right->isChecked();

    m_centre->setEnabled(!isHandEdited);
    m_right->setEnabled(!isHandEdited);
    m_lines->setEnabled(!isHandEdited && hasRoomForText);
    m_replace->setVisible(isHandEdited);

    QString status;
    if (isHandEdited)
    {
        status = QStringLiteral("%1 Saving leaves it as it is.").arg(m_problem);
    }
    else if (!m_problem.isEmpty())
    {
        status = QStringLiteral("Save replaces it with what the form shows.");
    }
    else if (!m_saved)
    {
        status = QStringLiteral("This project has no template.xml, which the Vita needs to install it, so Save "
                                "writes one.");
    }
    else if (!hasRoomForText)
    {
        status = QStringLiteral("A centred start button leaves no room for text.");
    }
    m_status->setText(status);
    m_status->setVisible(!status.isEmpty());
}
