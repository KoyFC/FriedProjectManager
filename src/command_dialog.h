#pragma once

#include <QDialog>
#include <QList>
#include <QProcess>
#include <QStringList>

class QDialogButtonBox;
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

    bool succeeded() const;

private:
    void runNext();
    void finish(const QString &message);
    void readOutput();
    void stop();
    void startFailed(QProcess::ProcessError error);
    void commandFinished(int exitCode, QProcess::ExitStatus status);
    void append(const QString &text);

    QPlainTextEdit *m_output = nullptr;
    QDialogButtonBox *m_buttons = nullptr;
    QPushButton *m_button = nullptr;
    QPushButton *m_folder = nullptr;
    QString m_folderPath;
    QProcess m_process;
    QList<QStringList> m_commands;
    int m_next = 0;
    bool m_succeeded = false;
};
