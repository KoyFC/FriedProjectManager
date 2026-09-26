#include "main_window.h"

#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

namespace
{
    const QString s_applicationTitle = QStringLiteral("Fried Project Manager");

    enum Platform
    {
        PlatformPc,
        PlatformVita
    };
}

MainWindow::MainWindow()
{
    setWindowTitle(s_applicationTitle);
    resize(900, 600);

    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&Open Project..."), QKeySequence::Open, this, &MainWindow::chooseProject);
    fileMenu->addAction(QStringLiteral("&Save"), QKeySequence::Save, this, &MainWindow::saveProject);

    buildForm();
    statusBar()->showMessage(QStringLiteral("No project open"));
}

void MainWindow::buildForm()
{
    m_form = new QWidget(this);

    QVBoxLayout *column = new QVBoxLayout(m_form);
    column->addWidget(buildPlatformRow());

    m_fields = new QFormLayout();
    column->addLayout(m_fields);
    column->addStretch();

    m_name = addField(QStringLiteral("Name"));
    m_organization = addField(QStringLiteral("Organization"));
    m_version = addField(QStringLiteral("Version"));
    m_windowTitle = addField(QStringLiteral("Window title"));
    m_vitaTitleId = addField(QStringLiteral("Vita title ID"));

    showPlatformFields();

    m_form->setEnabled(false);
    setCentralWidget(m_form);
}

QWidget *MainWindow::buildPlatformRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_platform = new QComboBox(row);
    m_platform->insertItem(PlatformPc, QStringLiteral("PC"));
    m_platform->insertItem(PlatformVita, QStringLiteral("PlayStation Vita"));

    layout->addWidget(new QLabel(QStringLiteral("Platform"), row));
    layout->addWidget(m_platform);
    layout->addStretch();

    connect(m_platform, &QComboBox::currentIndexChanged, this, &MainWindow::showPlatformFields);
    return row;
}

QLineEdit *MainWindow::addField(const QString &label)
{
    QLineEdit *field = new QLineEdit(m_form);
    m_fields->addRow(label, field);
    return field;
}

void MainWindow::showPlatformFields()
{
    const bool vita = m_platform->currentIndex() == PlatformVita;
    m_fields->setRowVisible(m_vitaTitleId, vita);
}

void MainWindow::chooseProject()
{
    const QString startAt = m_project.directory().isEmpty()
                                ? QDir::homePath()
                                : QDir(m_project.directory()).filePath(QStringLiteral(".."));

    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Open Project"), startAt);
    if (directory.isEmpty())
    {
        return;
    }

    openProject(directory);
}

bool MainWindow::openProject(const QString &directory)
{
    Project project;
    QString error;
    if (!project.load(directory, &error))
    {
        QMessageBox::warning(this, QStringLiteral("Open Project"), error);
        return false;
    }

    m_project = project;
    showProject();
    return true;
}

void MainWindow::showProject()
{
    m_name->setText(m_project.name());
    m_organization->setText(m_project.organization());
    m_version->setText(m_project.version());
    m_windowTitle->setText(m_project.windowTitle());
    m_vitaTitleId->setText(m_project.vitaTitleId());

    m_form->setEnabled(true);
    setWindowTitle(QStringLiteral("%1 - %2").arg(m_project.name(), s_applicationTitle));
    statusBar()->showMessage(QDir::toNativeSeparators(m_project.filePath()));
}

QString MainWindow::firstProblem(QLineEdit **field) const
{
    const QVector<QPair<QLineEdit *, QString>> problems = {
        {m_name, Project::checkIdentity(m_name->text())},
        {m_organization, Project::checkIdentity(m_organization->text())},
        {m_version, Project::checkVersion(m_version->text())},
        {m_vitaTitleId, Project::checkVitaTitleId(m_vitaTitleId->text())},
    };

    for (const auto &[edit, problem] : problems)
    {
        if (problem.isEmpty())
        {
            continue;
        }

        *field = edit;
        const QLabel *label = qobject_cast<QLabel *>(m_fields->labelForField(edit));
        return QStringLiteral("%1 %2").arg(label->text(), problem);
    }

    return QString();
}

bool MainWindow::saveProject()
{
    QLineEdit *invalid = nullptr;
    const QString problem = firstProblem(&invalid);
    if (!problem.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Save"), problem);
        invalid->setFocus();
        invalid->selectAll();
        return false;
    }

    m_project.setName(m_name->text());
    m_project.setOrganization(m_organization->text());
    m_project.setVersion(m_version->text());
    m_project.setWindowTitle(m_windowTitle->text());
    m_project.setVitaTitleId(m_vitaTitleId->text());

    QString error;
    if (!m_project.save(&error))
    {
        QMessageBox::warning(this, QStringLiteral("Save"), error);
        return false;
    }

    setWindowTitle(QStringLiteral("%1 - %2").arg(m_project.name(), s_applicationTitle));
    statusBar()->showMessage(QStringLiteral("Saved %1").arg(QDir::toNativeSeparators(m_project.filePath())));
    return true;
}
