#include "main_window.h"

#include "build.h"
#include "command_dialog.h"
#include "home_page.h"
#include "icon.h"
#include "icon_dialog.h"
#include "new_project_dialog.h"
#include "project_template.h"
#include "recent_projects.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QKeySequence>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    const QString s_applicationTitle = QStringLiteral("Fried Project Manager");
    const QString s_gitNoticeKey = QStringLiteral("newProject/showGitNotice");

    constexpr int s_iconPreviewSize = 32;
    constexpr int s_maxDisplaySide = 4096;

    // Stands for the project's own directory, so the field is one editable path
    // whichever side of the project it points at.
    const QString s_projectToken = QStringLiteral("{project}");

    int comboIndexOf(Platform platform)
    {
        return static_cast<int>(platform);
    }

    int comboIndexOf(Build::Type type)
    {
        return static_cast<int>(type);
    }

    // A project path holds separators, so it is percent encoded to stay one key.
    QString rememberedKey(const QString &setting, const QString &project, Platform platform)
    {
        return QStringLiteral("%1/%2/%3")
            .arg(setting, platformKey(platform), QString::fromUtf8(QUrl::toPercentEncoding(project)));
    }

    QString buildDirectoryKey(const QString &project, Platform platform)
    {
        return rememberedKey(QStringLiteral("buildDirectories"), project, platform);
    }

    QString buildTypeKey(const QString &project, Platform platform)
    {
        return rememberedKey(QStringLiteral("buildTypes"), project, platform);
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

    m_close = new QAction(QStringLiteral("&Close Project"), this);
    m_close->setShortcut(QKeySequence::Close);
    m_close->setEnabled(false);
    connect(m_close, &QAction::triggered, this, &MainWindow::showHome);
    fileMenu->addAction(m_close);
    fileMenu->addSeparator();

    m_icon = new QAction(QStringLiteral("Set &Icon..."), this);
    m_icon->setEnabled(false);
    connect(m_icon, &QAction::triggered, this, &MainWindow::chooseIcon);
    fileMenu->addAction(m_icon);

    m_build = new QAction(QStringLiteral("&Build"), this);
    m_build->setShortcut(QKeySequence(QStringLiteral("Ctrl+B")));
    m_build->setEnabled(false);
    connect(m_build, &QAction::triggered, this, &MainWindow::buildProject);
    menuBar()->addMenu(QStringLiteral("&Build"))->addAction(m_build);

    buildPages();
    showHome();
}

void MainWindow::buildPages()
{
    m_home = new HomePage(this);
    connect(m_home, &HomePage::projectChosen, this, &MainWindow::openProject);
    connect(m_home, &HomePage::newProjectRequested, this, &MainWindow::newProject);
    connect(m_home, &HomePage::openProjectRequested, this, &MainWindow::chooseProject);

    buildForm();

    m_pages = new QStackedWidget(this);
    m_pages->addWidget(m_home);
    m_pages->addWidget(m_form);
    setCentralWidget(m_pages);
}

void MainWindow::buildForm()
{
    m_form = new QWidget(this);

    QVBoxLayout *column = new QVBoxLayout(m_form);

    m_fields = new QFormLayout();
    column->addLayout(m_fields);
    column->addStretch();

    m_fields->addRow(QStringLiteral("Platform"), buildPlatformRow());
    m_fields->addRow(QStringLiteral("Icon"), buildIconRow());
    m_fields->addRow(QStringLiteral("Build type"), buildTypeRow());
    m_fields->addRow(QStringLiteral("Build directory"), buildDirectoryRow());

    m_buildWarning = new QLabel(m_form);
    m_buildWarning->setWordWrap(true);
    m_fields->addRow(QString(), m_buildWarning);

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
    m_fields->addRow(QStringLiteral("Display"), buildDisplayRow());

    m_ownDisplay = new QCheckBox(m_form);
    m_fields->addRow(QString(), m_ownDisplay);
    connect(m_ownDisplay, &QCheckBox::toggled, this, &MainWindow::ownDisplayToggled);

    showPlatformFields();

}

QWidget *MainWindow::buildPlatformRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_platform = new QComboBox(row);
    m_platform->insertItem(comboIndexOf(Platform::Pc), QStringLiteral("PC"));
    m_platform->insertItem(comboIndexOf(Platform::Vita), QStringLiteral("PlayStation Vita"));
    m_platform->insertItem(comboIndexOf(Platform::Switch), QStringLiteral("Nintendo Switch"));
    m_platform->insertItem(comboIndexOf(Platform::Nintendo3ds), QStringLiteral("Nintendo 3DS"));

    QToolButton *build = new QToolButton(row);
    build->setDefaultAction(m_build);

    layout->addWidget(m_platform);
    layout->addWidget(build);
    layout->addStretch();

    connect(m_platform, &QComboBox::currentIndexChanged, this, &MainWindow::showPlatformFields);
    return row;
}

QWidget *MainWindow::buildIconRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_iconPreview = new QLabel(row);
    m_iconPreview->setFixedSize(s_iconPreviewSize, s_iconPreviewSize);
    m_iconPreview->setAlignment(Qt::AlignCenter);
    m_iconPreview->setFrameShape(QFrame::StyledPanel);
    m_iconPreview->setToolTip(Icon::path(Platform::Pc));

    QPushButton *change = new QPushButton(QStringLiteral("Change..."), row);

    layout->addWidget(m_iconPreview);
    layout->addWidget(change);
    layout->addStretch();

    connect(change, &QPushButton::clicked, this, &MainWindow::chooseIcon);
    return row;
}

QWidget *MainWindow::buildTypeRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_buildType = new QComboBox(row);
    m_buildType->insertItem(comboIndexOf(Build::Type::Debug), Build::name(Build::Type::Debug));
    m_buildType->insertItem(comboIndexOf(Build::Type::Release), Build::name(Build::Type::Release));
    m_buildType->setToolTip(QStringLiteral("Debug keeps the symbols a debugger needs. Release optimises, which is "
                                           "what a build for players wants."));

    layout->addWidget(m_buildType);
    layout->addStretch();

    connect(m_buildType, &QComboBox::currentIndexChanged, this, &MainWindow::rememberBuildType);
    return row;
}

QWidget *MainWindow::buildDirectoryRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_buildDirectory = new QLineEdit(row);

    QPushButton *browse = new QPushButton(QStringLiteral("Browse..."), row);

    m_buildReset = new QPushButton(QStringLiteral("Reset"), row);
    m_buildReset->setToolTip(QStringLiteral("Back to where the chosen platform builds by default."));

    layout->addWidget(m_buildDirectory);
    layout->addWidget(browse);
    layout->addWidget(m_buildReset);

    connect(browse, &QPushButton::clicked, this, &MainWindow::chooseBuildDirectory);
    connect(m_buildReset, &QPushButton::clicked, this, &MainWindow::resetBuildDirectory);
    connect(m_buildDirectory, &QLineEdit::editingFinished, this, &MainWindow::rememberBuildDirectory);
    connect(m_buildDirectory, &QLineEdit::textChanged, this, &MainWindow::buildDirectoryChanged);
    return row;
}

QWidget *MainWindow::buildDisplayRow()
{
    QWidget *row = new QWidget(m_form);
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);

    m_displayMode = new QComboBox(row);
    const QList<std::array<QString, 3>> modes = {
        {QStringLiteral("Native resolution"), QStringLiteral("default"),
         QStringLiteral("Draws at the screen's own resolution, unscaled. The game adapts to whatever size it gets.")},
        {QStringLiteral("Fit"), QStringLiteral("fit"),
         QStringLiteral("Scales the game's size as large as the screen allows, keeping its shape, with black bars.")},
        {QStringLiteral("Integer scale"), QStringLiteral("integer"),
         QStringLiteral("Like Fit, but only by whole multiples, so every pixel stays the same size.")},
        {QStringLiteral("Expand"), QStringLiteral("expand"),
         QStringLiteral("Like Fit, but the game sees more on the longer side instead of black bars.")},
        {QStringLiteral("Stretch"), QStringLiteral("stretch"),
         QStringLiteral("Fills the screen with the game's size, distorting its shape.")},
    };
    for (const auto &[label, mode, description] : modes)
    {
        m_displayMode->addItem(label, mode);
        m_displayMode->setItemData(m_displayMode->count() - 1, description, Qt::ToolTipRole);
    }

    m_displayWidth = new QSpinBox(row);
    m_displayWidth->setRange(1, s_maxDisplaySide);
    m_displayWidth->setToolTip(QStringLiteral("The width the game is designed for."));
    m_displayHeight = new QSpinBox(row);
    m_displayHeight->setRange(1, s_maxDisplaySide);
    m_displayHeight->setToolTip(QStringLiteral("The height the game is designed for."));

    m_displayFilter = new QComboBox(row);
    m_displayFilter->addItem(QStringLiteral("Nearest"), QStringLiteral("nearest"));
    m_displayFilter->setItemData(0, QStringLiteral("Keeps pixels sharp when scaled, which is what pixel art wants."),
                                 Qt::ToolTipRole);
    m_displayFilter->addItem(QStringLiteral("Linear"), QStringLiteral("linear"));
    m_displayFilter->setItemData(1, QStringLiteral("Blends pixels when scaled, which suits art drawn at a high resolution."),
                                 Qt::ToolTipRole);

    layout->addWidget(m_displayMode);
    layout->addWidget(m_displayWidth);
    layout->addWidget(new QLabel(QStringLiteral("x"), row));
    layout->addWidget(m_displayHeight);
    layout->addWidget(m_displayFilter);
    layout->addStretch();

    connect(m_displayMode, &QComboBox::currentIndexChanged, this, &MainWindow::displayEdited);
    connect(m_displayWidth, &QSpinBox::valueChanged, this, &MainWindow::displayEdited);
    connect(m_displayHeight, &QSpinBox::valueChanged, this, &MainWindow::displayEdited);
    connect(m_displayFilter, &QComboBox::currentIndexChanged, this, &MainWindow::displayEdited);
    return row;
}

void MainWindow::showHome()
{
    if (m_pages->currentWidget() == m_form && !confirmLeavingProject())
    {
        return;
    }

    m_project = Project();
    m_build->setEnabled(false);
    m_icon->setEnabled(false);
    m_close->setEnabled(false);

    m_home->refresh();
    m_pages->setCurrentWidget(m_home);
    setWindowTitle(s_applicationTitle);
    statusBar()->showMessage(QStringLiteral("No project open"));
}

bool MainWindow::confirmLeavingProject()
{
    if (!hasUnsavedEdits())
    {
        return true;
    }

    QMessageBox ask(QMessageBox::Question, QStringLiteral("Close Project"),
                    QStringLiteral("The form holds edits that project.fried does not."), QMessageBox::Cancel, this);
    QPushButton *save = ask.addButton(QStringLiteral("Save and Close"), QMessageBox::AcceptRole);
    QPushButton *discard = ask.addButton(QStringLiteral("Discard"), QMessageBox::DestructiveRole);
    ask.setDefaultButton(save);
    ask.exec();

    if (ask.clickedButton() == save)
    {
        return saveProject();
    }

    return ask.clickedButton() == discard;
}

void MainWindow::chooseIcon()
{
    IconDialog dialog(this, m_project.directory());
    if (dialog.exec() == QDialog::Accepted)
    {
        showIcon();
        QMessageBox::information(this, s_applicationTitle, dialog.report());
    }
}

void MainWindow::showIcon()
{
    const QImage icon(QDir(m_project.directory()).filePath(Icon::path(Platform::Pc)));
    m_iconPreview->setPixmap(QPixmap::fromImage(icon).scaled(s_iconPreviewSize, s_iconPreviewSize, Qt::KeepAspectRatio,
                                                             Qt::SmoothTransformation));
}

void MainWindow::chooseBuildDirectory()
{
    const QDir project(m_project.directory());
    const QString chosen = QFileDialog::getExistingDirectory(this, QStringLiteral("Build Directory"),
                                                             project.filePath(chosenBuildDirectory()));
    if (chosen.isEmpty())
    {
        return;
    }

    // A directory inside the project travels with it, so it is kept relative.
    const QString relative = project.relativeFilePath(chosen);
    showBuildDirectory(relative.startsWith(QStringLiteral("..")) ? chosen : relative);
    rememberBuildDirectory();
}

void MainWindow::showBuildDirectory()
{
    QSettings settings;
    const QString key = buildDirectoryKey(m_project.directory(), selectedPlatform());
    showBuildDirectory(settings.value(key, Build::defaultDirectory(selectedPlatform())).toString());
}

void MainWindow::showBuildDirectory(const QString &directory)
{
    m_buildDirectory->setText(QDir::isAbsolutePath(directory)
                                  ? QDir::toNativeSeparators(directory)
                                  : s_projectToken + QChar('/') + directory);
}

void MainWindow::showBuildType()
{
    QSettings settings;
    const QString remembered =
        settings.value(buildTypeKey(m_project.directory(), selectedPlatform()), Build::name(Build::Type::Debug))
            .toString();
    m_buildType->setCurrentIndex(comboIndexOf(Build::typeNamed(remembered)));
}

void MainWindow::rememberBuildType()
{
    if (m_project.directory().isEmpty())
    {
        return;
    }

    QSettings settings;
    settings.setValue(buildTypeKey(m_project.directory(), selectedPlatform()), Build::name(selectedBuildType()));
}

Build::Type MainWindow::selectedBuildType() const
{
    return static_cast<Build::Type>(m_buildType->currentIndex());
}

void MainWindow::buildDirectoryChanged()
{
    const QString chosen = chosenBuildDirectory();
    m_buildDirectory->setToolTip(QDir(m_project.directory()).filePath(chosen));
    m_buildReset->setEnabled(chosen != Build::defaultDirectory(selectedPlatform()));

    const QString problem = vitaSpaceProblem();
    m_buildWarning->setText(problem);
    m_fields->setRowVisible(m_buildWarning, !problem.isEmpty());
}

void MainWindow::resetBuildDirectory()
{
    showBuildDirectory(Build::defaultDirectory(selectedPlatform()));
    rememberBuildDirectory();
}

// Whatever follows the token is relative to the project; anything else stands on its own.
QString MainWindow::chosenBuildDirectory() const
{
    QString typed = m_buildDirectory->text().trimmed();
    if (!typed.startsWith(s_projectToken))
    {
        return typed;
    }

    typed = typed.mid(s_projectToken.size());
    while (typed.startsWith(QChar('/')) || typed.startsWith(QChar('\\')))
    {
        typed.remove(0, 1);
    }
    return typed;
}

// VitaSDK hands vita-pack-vpk every path unquoted, so a space splits an argument.
QString MainWindow::vitaSpaceProblem() const
{
    if (selectedPlatform() != Platform::Vita)
    {
        return QString();
    }

    QStringList spaced;

    // Its assets are packed straight from here, wherever the build tree is.
    if (m_project.directory().contains(QChar(' ')))
    {
        spaced << QStringLiteral("the project's own path");
    }
    if (chosenBuildDirectory().contains(QChar(' ')))
    {
        spaced << QStringLiteral("the build directory");
    }

    if (spaced.isEmpty())
    {
        return QString();
    }

    return QStringLiteral("A Vita build will compile but fail to be packaged: VitaSDK does not quote the paths it "
                          "packs with, and there is a space in %1.")
        .arg(spaced.join(QStringLiteral(" and in ")));
}

void MainWindow::rememberBuildDirectory()
{
    if (m_project.directory().isEmpty() || chosenBuildDirectory().isEmpty())
    {
        return;
    }

    QSettings settings;
    settings.setValue(buildDirectoryKey(m_project.directory(), selectedPlatform()), chosenBuildDirectory());
}

QLineEdit *MainWindow::addField(const QString &label)
{
    QLineEdit *field = new QLineEdit(m_form);
    m_fields->addRow(label, field);
    return field;
}

void MainWindow::showPlatformFields()
{
    const bool vita = selectedPlatform() == Platform::Vita;
    m_fields->setRowVisible(m_vitaTitleId, vita);
    showDisplay();

    if (!m_project.directory().isEmpty())
    {
        showIcon();
        showBuildType();
        showBuildDirectory();
    }

    buildDirectoryChanged();
}

void MainWindow::showDisplay()
{
    const std::optional<DisplaySettings> &own = platformDisplay();
    const DisplaySettings shown = own ? *own : m_sharedDisplay;

    m_isShowingDisplay = true;
    m_ownDisplay->setText(QStringLiteral("Different on %1").arg(m_platform->currentText()));
    m_ownDisplay->setChecked(own.has_value());
    m_displayMode->setCurrentIndex(std::max(0, m_displayMode->findData(shown.m_mode)));
    m_displayWidth->setValue(shown.m_width);
    m_displayHeight->setValue(shown.m_height);
    m_displayFilter->setCurrentIndex(std::max(0, m_displayFilter->findData(shown.m_filter)));
    m_isShowingDisplay = false;

    const bool isScaled = shown.m_mode != QStringLiteral("default");
    m_displayWidth->setEnabled(isScaled);
    m_displayHeight->setEnabled(isScaled);
}

void MainWindow::displayEdited()
{
    if (m_isShowingDisplay)
    {
        return;
    }

    std::optional<DisplaySettings> &own = platformDisplay();
    (own ? *own : m_sharedDisplay) = displayInForm();
    showDisplay();
}

// A platform's own display starts as a copy of the shared one.
void MainWindow::ownDisplayToggled(bool isOwn)
{
    if (m_isShowingDisplay)
    {
        return;
    }

    std::optional<DisplaySettings> &own = platformDisplay();
    own = isOwn ? std::optional<DisplaySettings>(m_sharedDisplay) : std::nullopt;
    showDisplay();
}

DisplaySettings MainWindow::displayInForm() const
{
    return {
        m_displayMode->currentData().toString(),
        m_displayWidth->value(),
        m_displayHeight->value(),
        m_displayFilter->currentData().toString(),
    };
}

std::optional<DisplaySettings> &MainWindow::platformDisplay()
{
    return m_platformDisplays[comboIndexOf(selectedPlatform())];
}

bool MainWindow::hasUnsavedDisplay() const
{
    if (m_sharedDisplay != m_project.display())
    {
        return true;
    }

    for (int index = 0; index < s_platformCount; ++index)
    {
        if (m_platformDisplays[index] != m_project.display(static_cast<Platform>(index)))
        {
            return true;
        }
    }
    return false;
}

Platform MainWindow::selectedPlatform() const
{
    return static_cast<Platform>(m_platform->currentIndex());
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
    RecentProjects::remember(directory);
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

    m_sharedDisplay = m_project.display();
    for (int index = 0; index < s_platformCount; ++index)
    {
        m_platformDisplays[index] = m_project.display(static_cast<Platform>(index));
    }
    showDisplay();

    showIcon();
    showBuildType();
    showBuildDirectory();

    m_pages->setCurrentWidget(m_form);
    m_build->setEnabled(true);
    m_icon->setEnabled(true);
    m_close->setEnabled(true);
    setWindowTitle(QStringLiteral("%1 - %2").arg(m_project.name(), s_applicationTitle));
    statusBar()->showMessage(QDir::toNativeSeparators(m_project.filePath()));
}

bool MainWindow::hasUnsavedEdits() const
{
    return m_name->text() != m_project.name()
           || m_organization->text() != m_project.organization()
           || m_version->text() != m_project.version()
           || m_windowTitle->text() != m_project.windowTitle()
           || m_vitaTitleId->text() != m_project.vitaTitleId()
           || hasUnsavedDisplay();
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
    rememberBuildType();

    QString error;
    const QList<QStringList> commands =
        Build::commands(selectedPlatform(), selectedBuildType(), chosenBuildDirectory(), &error);
    if (commands.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Build"), error);
        return;
    }

    CommandDialog build(this, QStringLiteral("Build %1 (%2)").arg(m_platform->currentText(), Build::name(selectedBuildType())),
                        m_project.directory(), commands);
    build.setActivity(QStringLiteral("Compiling"));
    build.offerToOpen(QStringLiteral("Open Build Folder"),
                      QDir(m_project.directory()).filePath(chosenBuildDirectory()));
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
    m_project.setDisplay(m_sharedDisplay);
    for (int index = 0; index < s_platformCount; ++index)
    {
        m_project.setDisplay(static_cast<Platform>(index), m_platformDisplays[index]);
    }

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
