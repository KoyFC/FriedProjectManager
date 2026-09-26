#include "command_dialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

CommandDialog::CommandDialog(QWidget *parent, const QString &title, const QString &workingDirectory,
                             const QList<QStringList> &commands)
    : QDialog(parent)
    , m_commands(commands)
{
    setWindowTitle(title);
    resize(700, 400);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_button = m_buttons->button(QDialogButtonBox::Close);
    m_button->setText(QStringLiteral("Cancel"));

    QVBoxLayout *column = new QVBoxLayout(this);
    column->addWidget(m_output);
    column->addWidget(m_buttons);

    m_process.setWorkingDirectory(workingDirectory);
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&m_process, &QProcess::readyRead, this, &CommandDialog::readOutput);
    connect(&m_process, &QProcess::finished, this, &CommandDialog::commandFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &CommandDialog::startFailed);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &CommandDialog::stop);

    // Queued so the dialog is on screen before the first command starts.
    QMetaObject::invokeMethod(this, &CommandDialog::runNext, Qt::QueuedConnection);
}

void CommandDialog::offerToOpen(const QString &label, const QString &directory)
{
    m_folderPath = directory;
    m_folder = m_buttons->addButton(label, QDialogButtonBox::ActionRole);
    m_folder->setEnabled(QFileInfo::exists(directory));

    connect(m_folder, &QPushButton::clicked, this,
            [this] { QDesktopServices::openUrl(QUrl::fromLocalFile(m_folderPath)); });
}

bool CommandDialog::succeeded() const
{
    return m_succeeded;
}

void CommandDialog::append(const QString &text)
{
    m_output->appendPlainText(text);
}

void CommandDialog::runNext()
{
    if (m_next == m_commands.size())
    {
        m_succeeded = true;
        finish(QStringLiteral("\nDone."));
        return;
    }

    QStringList command = m_commands.at(m_next);
    const QString program = command.takeFirst();

    append(QStringLiteral("$ %1 %2").arg(program, command.join(QChar(' '))));
    m_process.start(program, command);
}

void CommandDialog::finish(const QString &message)
{
    m_button->setText(QStringLiteral("Close"));
    append(message);

    if (m_folder != nullptr)
    {
        m_folder->setEnabled(QFileInfo::exists(m_folderPath));
    }
}

void CommandDialog::stop()
{
    if (m_process.state() != QProcess::NotRunning)
    {
        m_process.kill();
        m_process.waitForFinished();
    }

    reject();
}

// QProcess reports a program it could not launch here instead of through finished().
void CommandDialog::startFailed(QProcess::ProcessError error)
{
    if (error != QProcess::FailedToStart)
    {
        return;
    }

    finish(QStringLiteral("\nStopped: %1 could not be started. %2")
               .arg(m_commands.at(m_next).first(), m_process.errorString()));
}

void CommandDialog::readOutput()
{
    const QString text = QString::fromLocal8Bit(m_process.readAll()).trimmed();
    if (!text.isEmpty())
    {
        append(text);
    }
}

void CommandDialog::commandFinished(int exitCode, QProcess::ExitStatus status)
{
    readOutput();

    if (status != QProcess::NormalExit || exitCode != 0)
    {
        finish(QStringLiteral("\nStopped: %1 exited with %2.").arg(m_commands.at(m_next).first()).arg(exitCode));
        return;
    }

    ++m_next;
    runNext();
}
