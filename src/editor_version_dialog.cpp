#include "editor_version_dialog.h"

#include "editor_install.h"
#include "help_label.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFont>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    constexpr int s_versionRole = Qt::UserRole;
}

EditorVersionDialog::EditorVersionDialog(QWidget *parent)
    : QDialog(parent)
    , m_installed(EditorInstall::installedVersions())
{
    setWindowTitle(QStringLiteral("Fried Editor Versions"));
    resize(420, 380);

    m_status = new QLabel(QStringLiteral("Asking the editor repository for its releases..."), this);
    m_status->setWordWrap(true);

    m_list = new QListWidget(this);
    m_list->setEnabled(false);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_download = buttons->addButton(QStringLiteral("Download"), QDialogButtonBox::AcceptRole);
    m_download->setEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_status);
    layout->addWidget(m_list, 1);
    layout->addWidget(helpLabel(QStringLiteral("Each version is kept in its own folder in %1, so a new one installs "
                                               "beside the others. The newest installed is the one that opens "
                                               "projects, and the Storage window deletes the rest.")
                                    .arg(QDir::toNativeSeparators(EditorInstall::versionsDirectory())),
                                this));
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &EditorVersionDialog::selectionChanged);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] {
        if (m_download->isEnabled())
        {
            accept();
        }
    });

    connect(&m_listing, &QProcess::finished, this, &EditorVersionDialog::listingFinished);
    connect(&m_listing, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
        {
            listingFailed();
        }
    });

    // A repository that asks for credentials would otherwise wait on a terminal nobody sees.
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("GIT_TERMINAL_PROMPT"), QStringLiteral("0"));
    m_listing.setProcessEnvironment(environment);

    const QStringList command = EditorInstall::listRemoteVersionsCommand();
    m_listing.start(command.first(), command.mid(1));
}

// Closed before the repository answered.
EditorVersionDialog::~EditorVersionDialog()
{
    if (m_listing.state() != QProcess::NotRunning)
    {
        m_listing.kill();
        m_listing.waitForFinished();
    }
}

QString EditorVersionDialog::chosenVersion() const
{
    const QList<QListWidgetItem *> selected = m_list->selectedItems();
    return selected.isEmpty() ? QString() : selected.first()->data(s_versionRole).toString();
}

void EditorVersionDialog::listingFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit || exitCode != 0)
    {
        listingFailed();
        return;
    }

    showVersions();
}

void EditorVersionDialog::listingFailed()
{
    const QString detail = QString::fromUtf8(m_listing.readAllStandardError()).trimmed();
    m_status->setText(QStringLiteral("The editor repository could not be reached, so there are no releases to "
                                     "choose from. Git has to be installed.%1")
                          .arg(detail.isEmpty() ? QString() : QStringLiteral("\n\n") + detail));
}

void EditorVersionDialog::showVersions()
{
    const QStringList versions = EditorInstall::parseRemoteVersions(QString::fromUtf8(m_listing.readAllStandardOutput()));
    if (versions.isEmpty())
    {
        m_status->setText(QStringLiteral("The editor repository has no releases yet."));
        return;
    }

    m_status->setText(QStringLiteral("Choose a release to download."));
    for (const QString &version : versions)
    {
        const bool installed = m_installed.contains(version);
        QListWidgetItem *item = new QListWidgetItem(
            installed ? QStringLiteral("%1  (installed)").arg(version) : version, m_list);
        item->setData(s_versionRole, version);
        if (installed)
        {
            QFont bold = item->font();
            bold.setBold(true);
            item->setFont(bold);
        }
    }

    // The newest one that is not installed yet, when there is one.
    for (int row = 0; row < m_list->count(); ++row)
    {
        if (!m_installed.contains(m_list->item(row)->data(s_versionRole).toString()))
        {
            m_list->setCurrentRow(row);
            break;
        }
    }
    m_list->setEnabled(true);
}

void EditorVersionDialog::selectionChanged()
{
    m_download->setEnabled(!chosenVersion().isEmpty() && !m_installed.contains(chosenVersion()));
}
