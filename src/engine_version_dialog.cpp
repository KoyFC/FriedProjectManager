#include "engine_version_dialog.h"

#include "help_label.h"

#include <QDialogButtonBox>
#include <QFont>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    constexpr int s_versionIndexRole = Qt::UserRole;
}

EngineVersionDialog::EngineVersionDialog(QWidget *parent, const QString &projectDirectory)
    : QDialog(parent)
    , m_currentCommit(EngineSubmodule::currentCommit(projectDirectory))
{
    setWindowTitle(QStringLiteral("Engine Version"));
    resize(420, 420);

    m_status = new QLabel(QStringLiteral("Asking the engine repository for its versions..."), this);
    m_status->setWordWrap(true);

    m_list = new QListWidget(this);
    m_list->setEnabled(false);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    m_switch = buttons->addButton(QStringLiteral("Switch"), QDialogButtonBox::AcceptRole);
    m_switch->setEnabled(false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_status);
    layout->addWidget(m_list, 1);
    layout->addWidget(helpLabel(QStringLiteral("The project's repository records the new version once the change "
                                               "to engine is committed."),
                                this));
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_list, &QListWidget::itemSelectionChanged, this, &EngineVersionDialog::selectionChanged);
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this] {
        if (m_switch->isEnabled())
        {
            accept();
        }
    });

    connect(&m_listing, &QProcess::finished, this, &EngineVersionDialog::listingFinished);
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

    const QStringList command = EngineSubmodule::listRemoteVersionsCommand();
    m_listing.setWorkingDirectory(EngineSubmodule::directory(projectDirectory));
    m_listing.start(command.first(), command.mid(1));
}

// Closed before the repository answered.
EngineVersionDialog::~EngineVersionDialog()
{
    if (m_listing.state() != QProcess::NotRunning)
    {
        m_listing.kill();
        m_listing.waitForFinished();
    }
}

EngineVersion EngineVersionDialog::chosenVersion() const
{
    const QList<QListWidgetItem *> selected = m_list->selectedItems();
    if (selected.isEmpty())
    {
        return EngineVersion();
    }

    return m_versions.value(selected.first()->data(s_versionIndexRole).toInt());
}

void EngineVersionDialog::listingFinished(int exitCode, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit || exitCode != 0)
    {
        listingFailed();
        return;
    }

    m_versions = EngineSubmodule::parseRemoteVersions(QString::fromUtf8(m_listing.readAllStandardOutput()));
    showVersions();
}

void EngineVersionDialog::listingFailed()
{
    const QString detail = QString::fromUtf8(m_listing.readAllStandardError()).trimmed();
    m_status->setText(QStringLiteral("The engine repository could not be reached, so there are no versions to "
                                     "choose from.%1")
                          .arg(detail.isEmpty() ? QString() : QStringLiteral("\n\n") + detail));
}

void EngineVersionDialog::showVersions()
{
    if (m_versions.isEmpty())
    {
        m_status->setText(QStringLiteral("The engine repository has no branches or tags."));
        return;
    }

    m_status->setText(QStringLiteral("Branches move as the engine is developed; a tag stays where it was released."));

    for (int index = 0; index < m_versions.size(); ++index)
    {
        const EngineVersion &version = m_versions[index];
        const QString kind =
            version.kind == EngineVersion::Kind::Branch ? QStringLiteral("branch") : QStringLiteral("tag");

        QListWidgetItem *item = new QListWidgetItem(QStringLiteral("%1  (%2)").arg(version.name, kind), m_list);
        item->setData(s_versionIndexRole, index);

        if (version.commit == m_currentCommit)
        {
            item->setText(item->text() + QStringLiteral(", current"));
            QFont bold = item->font();
            bold.setBold(true);
            item->setFont(bold);
        }
    }

    m_list->setEnabled(true);
}

void EngineVersionDialog::selectionChanged()
{
    const EngineVersion chosen = chosenVersion();
    m_switch->setEnabled(!chosen.name.isEmpty() && chosen.commit != m_currentCommit);
}
