#include "main_window.h"

#include "build.h"
#include "command_dialog.h"
#include "new_project_dialog.h"
#include "project_template.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStatusBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    const QString s_applicationTitle = QStringLiteral("Fried Project Manager");
    const QString s_gitNoticeKey = QStringLiteral("newProject/showGitNotice");

    // A project path holds separators, so it is percent encoded to stay one key.
    QString buildDirectoryKey(const QString &project, int platform)
    {
        return QStringLiteral("buildDirectories/%1/%2")
            .arg(platform == PlatformVita ? QStringLiteral("vita") : QStringLiteral("pc"),
                 QString::fromUtf8(QUrl::toPercentEncoding(project)));
    }
}

MainWindow::MainWindow()
{
    setWindowTitle(s_applicationTitle);
    resize(900, 600);

    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&New Project..."), QKeySequence::New, this, &MainWindow::newProject);
    fileMenu->addAction(QStringLiteral("&Open Project..."), QKeySequence::Open, this, &MainWindow::chooseProject);
    fileMenu->addAction(QStringLiteral("&Save"), QKeySequence::Save, this, &MainWindow::saveProject);

    m_build = new QAction(QStringLiteral("&Build"), this);
    m_build->setShortcut(QKeySequence(QStringLiteral("Ctrl+B")));
    m_build->setEnabled(false);
    connect(m_build, &QAction::triggered, this, &MainWindow::buildProject);
    menuBar()->addMenu(QStringLiteral("&Build"))->addAction(m_build);

    buildForm();
    statusBar()->showMessage(QStringLiteral("No project open"));
}

void MainWindow::buildForm()
{
    m_form = new QWidget(this);

    QVBoxLayout *column = new QVBoxLayout(m_form);

    m_fields = new QFormLayout();
    column->addLayout(m_fields);
    column->addStretch();

    m_fields->addRow(QStringLiteral("Platform"), buildPlatformRow());
    m_fields->addRow(QStringLiteral("Build directory"), buildDirectoryRow());

    // Everything below the line is what project.fried holds.
    QFrame *separator = new QFrame(m_form);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    m_fields->addRow(separator);

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

    QToolButton *build = new QToolButton(row);
    build->setDefaultAction(m_build);

    layout->addWidget(m_platform);
    layout->addWidget(build);
    layout->addStretch();

    connect(m_platform, &QComboBox::currentIndexChanged, this, &MainWindow::showPlatformFields);
    return row;
}

QWidget *MainWindow::buildDirectoryRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_buildDirectory = new QLineEdit(row);
    QPushButton *browse = new QPushButton(QStringLiteral("Browse..."), row);

    layout->addWidget(m_buildDirectory);
    layout->addWidget(browse);

    connect(browse, &QPushButton::clicked, this, &MainWindow::chooseBuildDirectory);
    connect(m_buildDirectory, &QLineEdit::editingFinished, this, &MainWindow::rememberBuildDirectory);
    return row;
}

void MainWindow::chooseBuildDirectory()
{
    const QDir project(m_project.directory());
    const QString chosen = QFileDialog::getExistingDirectory(this, QStringLiteral("Build Directory"),
                                                             project.filePath(m_buildDirectory->text()));
    if (chosen.isEmpty())
    {
        return;
    }

    // A directory inside the project travels with it, so it is kept relative.
    const QString relative = project.relativeFilePath(chosen);
    m_buildDirectory->setText(relative.startsWith(QStringLiteral("..")) ? chosen : relative);
    rememberBuildDirectory();
}

void MainWindow::showBuildDirectory()
{
    QSettings settings;
    const QString key = buildDirectoryKey(m_project.directory(), m_platform->currentIndex());
    m_buildDirectory->setText(settings.value(key, Build::defaultDirectory(m_platform->currentIndex())).toString());
}

void MainWindow::rememberBuildDirectory()
{
    if (m_project.directory().isEmpty() || m_buildDirectory->text().isEmpty())
    {
        return;
    }

    QSettings settings;
    settings.setValue(buildDirectoryKey(m_project.directory(), m_platform->currentIndex()), m_buildDirectory->text());
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

    if (!m_project.directory().isEmpty())
    {
        showBuildDirectory();
    }
}

QString MainWindow::nearbyLocation() const
{
    if (m_project.directory().isEmpty())
    {
        return QDir::homePath();
    }

    return QDir::cleanPath(QDir(m_project.directory()).filePath(QStringLiteral("..")));
}

bool MainWindow::confirmGitNotice()
{
    QSettings settings;
    if (!settings.value(s_gitNoticeKey, true).toBool())
    {
        return true;
    }

    QMessageBox notice(QMessageBox::Information, QStringLiteral("New Project"),
                       QStringLiteral("A new project is a Git repository with Fried Engine as a submodule, so creating one "
                                      "runs git and clones the engine.\n\ngit must be installed, and it needs to be able "
                                      "to reach the engine repository."),
                       QMessageBox::Ok | QMessageBox::Cancel, this);
    QCheckBox *hide = new QCheckBox(QStringLiteral("Don't show this again"), &notice);
    notice.setCheckBox(hide);

    if (notice.exec() != QMessageBox::Ok)
    {
        return false;
    }

    if (hide->isChecked())
    {
        settings.setValue(s_gitNoticeKey, false);
    }

    return true;
}

void MainWindow::newProject()
{
    if (!confirmGitNotice())
    {
        return;
    }

    NewProjectDialog dialog(this, nearbyLocation());
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString error;
    if (!ProjectTemplate::write(dialog.directory(), dialog.name(), dialog.organization(), &error))
    {
        QMessageBox::warning(this, QStringLiteral("New Project"), error);
        return;
    }

    openProject(dialog.directory());

    CommandDialog repository(this, QStringLiteral("Git Repository"), dialog.directory(), ProjectTemplate::repositoryCommands());
    repository.exec();
    if (!repository.succeeded())
    {
        QMessageBox::warning(this, QStringLiteral("New Project"),
                             QStringLiteral("The project files are written, but its Git repository is unfinished. "
                                            "Fix whatever the output reported and run the commands again in %1.")
                                 .arg(QDir::toNativeSeparators(dialog.directory())));
    }
}

void MainWindow::chooseProject()
{
    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Open Project"), nearbyLocation());
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

    showBuildDirectory();

    m_form->setEnabled(true);
    m_build->setEnabled(true);
    setWindowTitle(QStringLiteral("%1 - %2").arg(m_project.name(), s_applicationTitle));
    statusBar()->showMessage(QDir::toNativeSeparators(m_project.filePath()));
}

bool MainWindow::hasUnsavedEdits() const
{
    return m_name->text() != m_project.name()
           || m_organization->text() != m_project.organization()
           || m_version->text() != m_project.version()
           || m_windowTitle->text() != m_project.windowTitle()
           || m_vitaTitleId->text() != m_project.vitaTitleId();
}

// The build reads project.fried, not the form.
bool MainWindow::confirmUnsavedEdits()
{
    if (!hasUnsavedEdits())
    {
        return true;
    }

    QMessageBox ask(QMessageBox::Question, QStringLiteral("Build"),
                    QStringLiteral("The form holds edits that project.fried does not."), QMessageBox::Cancel, this);
    QPushButton *save = ask.addButton(QStringLiteral("Save and Build"), QMessageBox::AcceptRole);
    QPushButton *anyway = ask.addButton(QStringLiteral("Build Anyway"), QMessageBox::DestructiveRole);
    ask.setDefaultButton(save);
    ask.exec();

    if (ask.clickedButton() == save)
    {
        return saveProject();
    }

    return ask.clickedButton() == anyway;
}

void MainWindow::buildProject()
{
    if (!confirmUnsavedEdits())
    {
        return;
    }

    rememberBuildDirectory();

    QString error;
    const QList<QStringList> commands = Build::commands(m_platform->currentIndex(), m_buildDirectory->text(), &error);
    if (commands.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Build"), error);
        return;
    }

    CommandDialog build(this, QStringLiteral("Build %1").arg(m_platform->currentText()), m_project.directory(), commands);
    build.exec();

    statusBar()->showMessage(build.succeeded()
                                 ? QStringLiteral("Built %1 for %2").arg(m_project.name(), m_platform->currentText())
                                 : QStringLiteral("Build for %1 did not finish").arg(m_platform->currentText()));
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
