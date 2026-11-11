#pragma once

#include "platform.h"

#include <QGroupBox>

class QLabel;
class QPlainTextEdit;

// What in a project's assets a platform cannot take, checked when its page is shown and on request.
class AssetPanel : public QGroupBox
{
    Q_OBJECT

public:
    AssetPanel(Platform platform, QWidget *parent);

    // An empty directory means no project is open.
    void setProjectDirectory(const QString &directory);
    void refresh();

private:
    Platform m_platform;
    QString m_projectDirectory;
    QLabel *m_summary = nullptr;
    QPlainTextEdit *m_issues = nullptr;
};
