#include "toolchain_panel.h"

#include "build.h"
#include "build_bar.h"

#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    // Shown the way a shell takes it, so it can be pasted there.
    QString quoted(const QString &argument)
    {
        if (argument.isEmpty() || argument.contains(QChar(' ')))
        {
            return QStringLiteral("\"%1\"").arg(argument);
        }
        return argument;
    }
}

ToolchainPanel::ToolchainPanel(Platform platform, const BuildBar *buildBar, QWidget *parent)
    : QGroupBox(QStringLiteral("Toolchain"), parent)
    , m_platform(platform)
    , m_buildBar(buildBar)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    for (const BuildTarget target : buildTargets())
    {
        if (platformOf(target) != platform)
        {
            continue;
        }

        Row row;
        row.target = target;
        row.status = new QLabel(this);
        row.status->setWordWrap(true);
        row.download = new QPushButton(QStringLiteral("Download..."), this);
        row.download->hide();
        connect(row.download, &QPushButton::clicked, this, [this, target] { emit downloadRequested(target); });

        QHBoxLayout *line = new QHBoxLayout;
        line->addWidget(row.status, 1);
        line->addWidget(row.download);
        layout->addLayout(line);
        m_rows << row;
    }

    m_commandsCaption = new QLabel(this);
    m_commands = new QPlainTextEdit(this);
    m_commands->setReadOnly(true);
    m_commands->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_commands->setMaximumHeight(110);

    QPushButton *copy = new QPushButton(QStringLiteral("Copy Commands"), this);
    connect(copy, &QPushButton::clicked, this, &ToolchainPanel::copyCommands);

    layout->addWidget(m_commandsCaption);
    layout->addWidget(m_commands);
    layout->addWidget(copy, 0, Qt::AlignLeft);
}

void ToolchainPanel::setProjectDirectory(const QString &directory)
{
    m_projectDirectory = directory;
    refresh();
}

void ToolchainPanel::refresh()
{
    for (Row &row : m_rows)
    {
        const bool needsDownload = Build::needsCompilerDownload(row.target) || Build::needsEmscriptenDownload(row.target);

        QString error;
        const QList<QStringList> commands = Build::commands(row.target, Build::Type::Debug, false, m_projectDirectory,
                                                            Build::defaultDirectory(row.target), &error);
        QString text;
        if (needsDownload)
        {
            text = QStringLiteral("Not ready: the tool has to download a compiler first.");
        }
        else if (commands.isEmpty())
        {
            text = QStringLiteral("Not ready: %1").arg(error);
        }
        else
        {
            text = QStringLiteral("Ready");
        }

        row.status->setText(m_rows.size() > 1 ? QStringLiteral("%1: %2").arg(buildTargetName(row.target), text) : text);
        row.download->setVisible(needsDownload);
    }

    // The commands are those of the target the build bar has on this platform, with the bar's own settings.
    const BuildTarget target = m_buildBar->target();
    if (platformOf(target) != m_platform || m_projectDirectory.isEmpty())
    {
        m_commandsCaption->setText(QStringLiteral("Choose this platform to see the commands its build runs."));
        m_commands->clear();
        return;
    }

    QString error;
    const QList<QStringList> commands = Build::commands(target, m_buildBar->type(), m_buildBar->logToFile(),
                                                        m_projectDirectory, m_buildBar->directory(), &error);
    m_commandsCaption->setText(QStringLiteral("Commands for %1 (%2), run in the project folder:")
                                   .arg(buildTargetName(target), Build::name(m_buildBar->type())));
    if (commands.isEmpty())
    {
        m_commands->setPlainText(error);
        return;
    }

    QStringList lines;
    for (const QStringList &command : commands)
    {
        QStringList parts;
        for (const QString &argument : command)
        {
            parts << quoted(argument);
        }
        lines << parts.join(QChar(' '));
    }
    m_commands->setPlainText(lines.join(QChar('\n')));
}

void ToolchainPanel::copyCommands()
{
    QGuiApplication::clipboard()->setText(m_commands->toPlainText());
}
