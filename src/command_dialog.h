#pragma once

#include <QDialog>
#include <QList>
#include <QProcess>
#include <QStringList>
#include <QTextCharFormat>
#include <QTimer>

class QDialogButtonBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    // Each command is a program followed by its arguments.
    CommandDialog(QWidget *parent, const QString &title, const QString &workingDirectory, const QList<QStringList> &commands);

    // Adds a button that opens this directory, for when the commands leave something to look at.
    void offerToOpen(const QString &label, const QString &directory);

    // What the commands are doing, for the line that reports they still are.
    void setActivity(const QString &activity);

    bool succeeded() const;

private:
    // What a line of output is. The dialog's own lines say which they are; the
    // tools' lines are judged by what they print.
    enum class Line
    {
        Plain,
        Command,
        Progress,
        Warning,
        Error,
        Success
    };

    void runNext();
    void finish(const QString &message, Line kind);
    void readOutput();
    void stop();
    void startFailed(QProcess::ProcessError error);
    void commandFinished(int exitCode, QProcess::ExitStatus status);
    void append(const QString &text, Line kind);

    // Splits a chunk of output into lines, holding back an unterminated tail.
    void appendOutput(const QString &chunk);
    void flushOutput();

    static Line kindOf(const QString &line);
    static QTextCharFormat formatFor(Line kind, const QPalette &palette);

    void showStatus(const QString &text, Line kind);
    void showRunning();
    void tickStatus();

    // Wide enough for the longest status it will ever hold, so the text does not
    // move as the dots come and go.
    void reserveStatusWidth();

    QPlainTextEdit *m_output = nullptr;
    QLabel *m_status = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QPushButton *m_button = nullptr;
    QPushButton *m_folder = nullptr;
    QString m_folderPath;
    QProcess m_process;
    QList<QStringList> m_commands;
    QString m_pending;
    QString m_activity;
    QTimer m_statusTick;
    int m_dots = 0;
    int m_next = 0;
    bool m_succeeded = false;
};
