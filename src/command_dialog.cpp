#include "command_dialog.h"

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextCursor>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    // Halfway to the background, so progress and status lines recede without
    // becoming unreadable on either a light or a dark theme.
    QColor recededIn(const QPalette &palette)
    {
        const QColor text = palette.color(QPalette::Text);
        const QColor base = palette.color(QPalette::Base);
        return QColor((text.red() + base.red()) / 2, (text.green() + base.green()) / 2,
                      (text.blue() + base.blue()) / 2);
    }

    // A hue dark enough to read on white is too dark to read on near-black, so
    // each one is picked from the theme actually in use.
    QColor onTheme(const QPalette &palette, const QColor &forLight, const QColor &forDark)
    {
        return palette.color(QPalette::Base).lightness() < 128 ? forDark : forLight;
    }

    constexpr int s_statusTickMs = 400;
    constexpr int s_mostDots = 3;

    const QString s_finishedStatus = QStringLiteral("Done.");
    const QString s_failedStatus = QStringLiteral("Failed.");
}

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

    // Left aligned inside a reserved width, so the words stay put while the dots
    // come and go, and the block as a whole sits in the middle.
    m_status = new QLabel(this);
    m_status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_button = m_buttons->button(QDialogButtonBox::Close);
    m_button->setText(QStringLiteral("Cancel"));

    QVBoxLayout *column = new QVBoxLayout(this);
    column->addWidget(m_output);
    column->addWidget(m_status, 0, Qt::AlignHCenter);
    column->addWidget(m_buttons);

    m_activity = QStringLiteral("Working");
    reserveStatusWidth();
    showRunning();

    m_statusTick.setInterval(s_statusTickMs);
    connect(&m_statusTick, &QTimer::timeout, this, &CommandDialog::tickStatus);
    m_statusTick.start();

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

void CommandDialog::setActivity(const QString &activity)
{
    m_activity = activity;
    reserveStatusWidth();

    // Nothing to refresh once the commands have reported how they ended.
    if (m_statusTick.isActive())
    {
        showRunning();
    }
}

void CommandDialog::reserveStatusWidth()
{
    const QFontMetrics metrics = m_status->fontMetrics();
    int widest = metrics.horizontalAdvance(m_activity + QString(s_mostDots, QChar('.')));
    widest = qMax(widest, metrics.horizontalAdvance(s_finishedStatus));
    widest = qMax(widest, metrics.horizontalAdvance(s_failedStatus));

    m_status->setMinimumWidth(widest);
}

void CommandDialog::showStatus(const QString &text, Line kind)
{
    m_status->setText(text);
    m_status->setStyleSheet(
        QStringLiteral("color: %1").arg(formatFor(kind, m_output->palette()).foreground().color().name()));
}

void CommandDialog::showRunning()
{
    showStatus(m_activity + QString(m_dots, QChar('.')), Line::Progress);
}

void CommandDialog::tickStatus()
{
    m_dots = (m_dots + 1) % (s_mostDots + 1);
    showRunning();
}

bool CommandDialog::succeeded() const
{
    return m_succeeded;
}

QTextCharFormat CommandDialog::formatFor(Line kind, const QPalette &palette)
{
    QTextCharFormat format;

    switch (kind)
    {
    case Line::Command:
        format.setForeground(palette.color(QPalette::Text));
        format.setFontWeight(QFont::Bold);
        break;
    case Line::Progress:
        format.setForeground(recededIn(palette));
        break;
    case Line::Warning:
        format.setForeground(onTheme(palette, QColor(0x8a, 0x61, 0x00), QColor(0xe0, 0xb0, 0x50)));
        break;
    case Line::Error:
        format.setForeground(onTheme(palette, QColor(0xb0, 0x00, 0x20), QColor(0xff, 0x6b, 0x6b)));
        break;
    case Line::Success:
        format.setForeground(onTheme(palette, QColor(0x1b, 0x7f, 0x3a), QColor(0x6d, 0xdc, 0x8b)));
        break;
    case Line::Plain:
        format.setForeground(palette.color(QPalette::Text));
        break;
    }

    return format;
}

// Every pattern here is something one of the three tools in the pipeline prints:
// gcc and clang diagnostics, CMake's own messages and progress, and haxe, whose
// errors carry no word for what they are and have to be recognised by shape.
CommandDialog::Line CommandDialog::kindOf(const QString &line)
{
    static const QRegularExpression haxeDiagnostic(QStringLiteral(R"(^\S+\.hx:\d+: )"));

    if (line.contains(QStringLiteral("error:"), Qt::CaseInsensitive)
        || line.contains(QStringLiteral("CMake Error"))
        || line.contains(QStringLiteral("undefined reference to"))
        || haxeDiagnostic.match(line).hasMatch())
    {
        return Line::Error;
    }

    if (line.contains(QStringLiteral("warning:"), Qt::CaseInsensitive)
        || line.contains(QStringLiteral("CMake Warning"))
        || line.startsWith(QStringLiteral("Warning :")))
    {
        return Line::Warning;
    }

    // make's percentage, and CMake's own status lines.
    if (line.startsWith(QStringLiteral("[")) || line.startsWith(QStringLiteral("-- ")))
    {
        return Line::Progress;
    }

    return Line::Plain;
}

void CommandDialog::append(const QString &text, Line kind)
{
    QScrollBar *scroll = m_output->verticalScrollBar();
    const bool wasAtBottom = scroll->value() == scroll->maximum();

    QTextCursor cursor(m_output->document());
    cursor.movePosition(QTextCursor::End);
    if (!m_output->document()->isEmpty())
    {
        cursor.insertBlock();
    }
    cursor.insertText(text, formatFor(kind, m_output->palette()));

    // Following the output is the point, but not at the cost of yanking the view
    // away from someone who scrolled up to read something.
    if (wasAtBottom)
    {
        scroll->setValue(scroll->maximum());
    }
}

// A read ends wherever the pipe ran dry rather than at a line break, so the tail
// of a chunk is usually half a line and waits for the rest of it.
void CommandDialog::appendOutput(const QString &chunk)
{
    m_pending += chunk;

    for (int newline = m_pending.indexOf(QChar('\n')); newline != -1; newline = m_pending.indexOf(QChar('\n')))
    {
        QString line = m_pending.left(newline);
        m_pending.remove(0, newline + 1);

        if (line.endsWith(QChar('\r')))
        {
            line.chop(1);
        }

        append(line, kindOf(line));
    }
}

void CommandDialog::flushOutput()
{
    if (m_pending.isEmpty())
    {
        return;
    }

    append(m_pending, kindOf(m_pending));
    m_pending.clear();
}

void CommandDialog::runNext()
{
    if (m_next == m_commands.size())
    {
        m_succeeded = true;
        finish(QStringLiteral("\nDone."), Line::Success);
        return;
    }

    QStringList command = m_commands.at(m_next);
    const QString program = command.takeFirst();

    append(QStringLiteral("$ %1 %2").arg(program, command.join(QChar(' '))), Line::Command);
    m_process.start(program, command);
}

void CommandDialog::finish(const QString &message, Line kind)
{
    m_statusTick.stop();
    showStatus(kind == Line::Success ? s_finishedStatus : s_failedStatus, kind);

    m_button->setText(QStringLiteral("Close"));
    append(message, kind);

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

    flushOutput();
    finish(QStringLiteral("\nStopped: %1 could not be started. %2")
               .arg(m_commands.at(m_next).first(), m_process.errorString()),
           Line::Error);
}

void CommandDialog::readOutput()
{
    appendOutput(QString::fromLocal8Bit(m_process.readAll()));
}

void CommandDialog::commandFinished(int exitCode, QProcess::ExitStatus status)
{
    readOutput();
    flushOutput();

    if (status != QProcess::NormalExit || exitCode != 0)
    {
        finish(QStringLiteral("\nStopped: %1 exited with %2.").arg(m_commands.at(m_next).first()).arg(exitCode),
               Line::Error);
        return;
    }

    ++m_next;
    runNext();
}
