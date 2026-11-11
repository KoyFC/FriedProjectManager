#include "asset_panel.h"

#include "asset_check.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

AssetPanel::AssetPanel(Platform platform, QWidget *parent)
    : QGroupBox(QStringLiteral("Assets"), parent)
    , m_platform(platform)
{
    m_summary = new QLabel(this);
    m_summary->setWordWrap(true);

    m_issues = new QPlainTextEdit(this);
    m_issues->setReadOnly(true);
    m_issues->setMaximumHeight(140);

    QPushButton *rescan = new QPushButton(QStringLiteral("Check Again"), this);
    rescan->setToolTip(QStringLiteral("Look at the project's assets again, after changing them."));
    connect(rescan, &QPushButton::clicked, this, &AssetPanel::refresh);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_summary);
    layout->addWidget(m_issues);
    layout->addWidget(rescan, 0, Qt::AlignLeft);
}

void AssetPanel::setProjectDirectory(const QString &directory)
{
    m_projectDirectory = directory;
    refresh();
}

void AssetPanel::refresh()
{
    if (m_projectDirectory.isEmpty())
    {
        m_summary->clear();
        m_issues->clear();
        return;
    }

    const QList<AssetIssue> issues = AssetCheck::validate(m_platform, m_projectDirectory);
    if (issues.isEmpty())
    {
        m_summary->setText(QStringLiteral("The assets suit this platform."));
        m_issues->clear();
        m_issues->hide();
        return;
    }

    int errors = 0;
    QStringList lines;
    for (const AssetIssue &issue : issues)
    {
        errors += issue.isError ? 1 : 0;
        const QString label = issue.isError ? QStringLiteral("Error") : QStringLiteral("Warning");
        lines << (issue.asset.isEmpty() ? QStringLiteral("%1: %2").arg(label, issue.message)
                                        : QStringLiteral("%1: %2: %3").arg(label, issue.asset, issue.message));
    }

    m_summary->setText(QStringLiteral("%1 errors and %2 warnings. This only reports: the Fried Editor's Issues tab "
                                      "can fix an oversized texture or a stereo sound.")
                           .arg(errors)
                           .arg(issues.size() - errors));
    m_issues->setPlainText(lines.join(QChar('\n')));
    m_issues->show();
}
