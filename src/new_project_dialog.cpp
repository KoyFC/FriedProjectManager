#include "new_project_dialog.h"

#include "project.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

namespace
{
    // The name is kept as it was typed; the directory it goes in drops the spaces.
    QString directoryName(const QString &name)
    {
        static const QRegularExpression whitespace(QStringLiteral("\\s+"));
        QString folder = name.trimmed();
        return folder.replace(whitespace, QStringLiteral("-"));
    }
}

NewProjectDialog::NewProjectDialog(QWidget *parent, const QString &location)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("New Project"));

    m_location = new QLineEdit(location, this);
    m_name = new QLineEdit(this);
    m_organization = new QLineEdit(this);

    QPushButton *browse = new QPushButton(QStringLiteral("Browse..."), this);

    QHBoxLayout *locationRow = new QHBoxLayout();
    locationRow->addWidget(m_location);
    locationRow->addWidget(browse);

    QFormLayout *fields = new QFormLayout();
    fields->addRow(QStringLiteral("Location"), locationRow);
    fields->addRow(QStringLiteral("Name"), m_name);
    fields->addRow(QStringLiteral("Organization"), m_organization);

    m_message = new QLabel(this);
    m_message->setWordWrap(true);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_create = buttons->button(QDialogButtonBox::Ok);
    m_create->setText(QStringLiteral("Create"));

    QVBoxLayout *column = new QVBoxLayout(this);
    column->addLayout(fields);
    column->addWidget(m_message);
    column->addWidget(buttons);

    connect(browse, &QPushButton::clicked, this, &NewProjectDialog::chooseLocation);
    connect(m_location, &QLineEdit::textChanged, this, &NewProjectDialog::refresh);
    connect(m_name, &QLineEdit::textChanged, this, &NewProjectDialog::refresh);
    connect(m_organization, &QLineEdit::textChanged, this, &NewProjectDialog::refresh);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refresh();
}

QString NewProjectDialog::directory() const
{
    return QDir::cleanPath(QDir(m_location->text()).filePath(directoryName(m_name->text())));
}

QString NewProjectDialog::name() const
{
    return m_name->text();
}

QString NewProjectDialog::organization() const
{
    return m_organization->text();
}

void NewProjectDialog::chooseLocation()
{
    const QString location = QFileDialog::getExistingDirectory(this, QStringLiteral("Project Location"), m_location->text());
    if (location.isEmpty())
    {
        return;
    }

    m_location->setText(location);
}

QString NewProjectDialog::problem() const
{
    if (m_location->text().isEmpty())
    {
        return QStringLiteral("Choose where the project directory goes.");
    }

    const QString nameProblem = Project::checkIdentity(m_name->text());
    if (!nameProblem.isEmpty())
    {
        return QStringLiteral("Name %1").arg(nameProblem);
    }

    const QString organizationProblem = Project::checkIdentity(m_organization->text());
    if (!organizationProblem.isEmpty())
    {
        return QStringLiteral("Organization %1").arg(organizationProblem);
    }

    if (directoryName(m_name->text()).isEmpty())
    {
        return QStringLiteral("Name must hold something other than spaces.");
    }

    const QDir target(directory());
    if (target.exists() && !target.isEmpty())
    {
        return QStringLiteral("%1 already exists and is not empty.").arg(QDir::toNativeSeparators(directory()));
    }

    return QString();
}

void NewProjectDialog::refresh()
{
    const QString found = problem();
    m_create->setEnabled(found.isEmpty());

    if (!found.isEmpty())
    {
        m_message->setText(found);
        return;
    }

    QString message = QStringLiteral("Creates %1").arg(QDir::toNativeSeparators(directory()));

    const QString folder = directoryName(m_name->text());
    if (folder != m_name->text())
    {
        // VitaSDK's packaging step does not quote the paths it is given.
        message += QStringLiteral("\n\nThe project keeps the name \"%1\", but its directory is \"%2\": a space in the "
                                  "path stops a Vita build from being packaged.")
                       .arg(m_name->text(), folder);
    }

    if (QFileInfo(directory()).path().contains(QChar(' ')))
    {
        message += QStringLiteral("\n\nThe location itself contains a space, so a Vita build will compile but fail to "
                                  "package. A PC build is unaffected.");
    }

    m_message->setText(message);
}
