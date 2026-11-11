#pragma once

#include "build_target.h"

#include <QGroupBox>

class BuildBar;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QVBoxLayout;

// Whether a platform can be built on this machine, and the commands the build bar would run, derived from the same calls a build makes.
class ToolchainPanel : public QGroupBox
{
    Q_OBJECT

public:
    ToolchainPanel(Platform platform, const BuildBar *buildBar, QWidget *parent);

    // An empty directory means no project is open.
    void setProjectDirectory(const QString &directory);
    void refresh();

signals:
    void downloadRequested(BuildTarget target);

private:
    struct Row
    {
        BuildTarget target;
        QLabel *status = nullptr;
        QPushButton *download = nullptr;
    };

    void copyCommands();

    Platform m_platform;
    const BuildBar *m_buildBar;
    QString m_projectDirectory;
    QList<Row> m_rows;
    QLabel *m_commandsCaption = nullptr;
    QPlainTextEdit *m_commands = nullptr;
};
