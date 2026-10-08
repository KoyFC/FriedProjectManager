#include "main_window.h"

#include "build.h"
#include "build_bar.h"
#include "command_dialog.h"
#include "display_editor.h"
#include "help_label.h"
#include "home_page.h"
#include "icon.h"
#include "icon_dialog.h"
#include "image_file_editor.h"
#include "live_area.h"
#include "live_area_template_editor.h"
#include "new_project_dialog.h"
#include "page_scroll_area.h"
#include "project_template.h"
#include "recent_projects.h"

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

#include <tuple>

namespace
{
    const QString s_applicationTitle = QStringLiteral("Fried Project Manager");
    const QString s_gitNoticeKey = QStringLiteral("newProject/showGitNotice");

    constexpr int s_sectionRole = Qt::UserRole;
    constexpr int s_sectionListWidth = 170;
    constexpr int s_sectionPadding = 4;
    constexpr int s_iconPreviewSize = 96;
    constexpr int s_iconColumnWidth = 140;

    // Every platform in the order the sidebar and the icons page list them.
    const QList<Platform> s_platforms = {Platform::Pc, Platform::Vita, Platform::Switch, Platform::Nintendo3ds};

    int indexOf(Platform platform)
    {
        return static_cast<int>(platform);
    }

}

MainWindow::MainWindow()
{
    setWindowTitle(s_applicationTitle);
    resize(960, 640);

    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&New Project..."), QKeySequence::New, this, &MainWindow::newProject);
    fileMenu->addAction(QStringLiteral("&Open Project..."), QKeySequence::Open, this, &MainWindow::chooseProject);

    m_save = fileMenu->addAction(QStringLiteral("&Save"), QKeySequence::Save, this, &MainWindow::saveProject);
    m_save->setEnabled(false);

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

    m_projectView = buildProjectView();

    m_pages = new QStackedWidget(this);
    m_pages->addWidget(m_home);
    m_pages->addWidget(m_projectView);
    setCentralWidget(m_pages);
}

// A sidebar of sections beside the selected one, with the build controls below both.
QWidget *MainWindow::buildProjectView()
{
    QWidget *view = new QWidget(this);

    m_sections = new QStackedWidget(view);

    // One panel holding two lists, so the heading between them is a label and not a row.
    QFrame *panel = new QFrame(view);
    panel->setFrameShape(QFrame::StyledPanel);
    panel->setBackgroundRole(QPalette::Base);
    panel->setAutoFillBackground(true);
    panel->setFixedWidth(s_sectionListWidth);

    QListWidget *project = newSectionList(panel);
    addSection(project, QStringLiteral("General"), buildGeneralPage());
    addSection(project, QStringLiteral("Display"), buildDisplayPage());
    addSection(project, QStringLiteral("Icons"), buildIconsPage());

    QListWidget *platforms = newSectionList(panel);
    for (const Platform platform : s_platforms)
    {
        addSection(platforms, platformName(platform), buildPlatformPage(platform));
    }

    QLabel *platformsHeading = helpLabel(QStringLiteral("PLATFORMS"), panel);
    QFont small = platformsHeading->font();
    small.setBold(true);
    small.setPointSizeF(small.pointSizeF() * 0.85);
    platformsHeading->setFont(small);
    platformsHeading->setContentsMargins(s_sectionPadding, 0, 0, 0);

    QVBoxLayout *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(s_sectionPadding, s_sectionPadding, s_sectionPadding, s_sectionPadding);
    panelLayout->addWidget(project);
    panelLayout->addSpacing(s_sectionPadding * 2);
    panelLayout->addWidget(platformsHeading);
    panelLayout->addWidget(platforms);
    panelLayout->addStretch();

    project->setCurrentRow(0);

    m_saveButton = new QPushButton(QStringLiteral("Save"), view);
    m_saveButton->setToolTip(QStringLiteral("Write the form to project.fried."));
    connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::saveProject);

    // The way back to the project list, without a trip to the File menu.
    QToolButton *back = new QToolButton(view);
    back->setText(QStringLiteral("Projects"));
    back->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
    back->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    back->setAutoRaise(true);
    back->setToolTip(QStringLiteral("Back to the project list (%1)")
                         .arg(m_close->shortcut().toString(QKeySequence::NativeText)));
    connect(back, &QToolButton::clicked, this, &MainWindow::showHome);

    QVBoxLayout *sidebar = new QVBoxLayout;
    sidebar->addWidget(back, 0, Qt::AlignLeft);
    sidebar->addWidget(panel, 1);
    sidebar->addWidget(m_saveButton);

    QHBoxLayout *body = new QHBoxLayout;
    body->addLayout(sidebar);
    body->addWidget(m_sections, 1);

    QFrame *separator = new QFrame(view);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    m_buildBar = new BuildBar(view, m_build);

    QVBoxLayout *layout = new QVBoxLayout(view);
    layout->addLayout(body, 1);
    layout->addWidget(separator);
    layout->addWidget(m_buildBar);
    return view;
}

QWidget *MainWindow::newPage(const QString &title, const QString &description, QVBoxLayout **content)
{
    QWidget *page = new QWidget;

    QLabel *heading = new QLabel(title, page);
    QFont large = heading->font();
    large.setPointSize(large.pointSize() + 4);
    large.setBold(true);
    heading->setFont(large);

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(heading);
    layout->addWidget(helpLabel(description, page));
    layout->addSpacing(heading->fontMetrics().lineSpacing() / 2);

    *content = new QVBoxLayout;
    layout->addLayout(*content);
    layout->addStretch();

    // A page that outgrows the window scrolls instead of squeezing its fields.
    return new PageScrollArea(page);
}

QListWidget *MainWindow::newSectionList(QWidget *parent)
{
    QListWidget *list = new QListWidget(parent);
    list->setFrameShape(QFrame::NoFrame);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setStyleSheet(QStringLiteral("QListWidget::item { padding: %1px; }").arg(s_sectionPadding));

    // Only one row is selected across every list, as if they were one. Each list
    // keeps its current row, since a list without one takes its first on focus.
    connect(list, &QListWidget::itemSelectionChanged, this, [this, list] {
        const QList<QListWidgetItem *> selected = list->selectedItems();
        if (selected.isEmpty())
        {
            return;
        }

        for (QListWidget *other : std::as_const(m_sectionLists))
        {
            if (other != list)
            {
                const QSignalBlocker blocker(other);
                other->clearSelection();
                other->viewport()->update();
            }
        }
        m_sections->setCurrentIndex(selected.first()->data(s_sectionRole).toInt());
    });

    m_sectionLists << list;
    return list;
}

void MainWindow::addSection(QListWidget *list, const QString &name, QWidget *page)
{
    QListWidgetItem *item = new QListWidgetItem(name, list);
    item->setData(s_sectionRole, m_sections->addWidget(page));

    // Every row shows at once, so the list is exactly as tall as its rows.
    list->setFixedHeight(list->sizeHintForRow(0) * list->count() + 2 * list->frameWidth());
}

void MainWindow::showSectionHolding(QWidget *widget)
{
    for (QWidget *ancestor = widget; ancestor; ancestor = ancestor->parentWidget())
    {
        const int index = m_sections->indexOf(ancestor);
        if (index < 0)
        {
            continue;
        }

        for (QListWidget *list : std::as_const(m_sectionLists))
        {
            for (int row = 0; row < list->count(); ++row)
            {
                if (list->item(row)->data(s_sectionRole).toInt() == index)
                {
                    list->setCurrentRow(row);
                    return;
                }
            }
        }
    }
}

QLineEdit *MainWindow::addField(QFormLayout *form, QWidget *parent, const QString &label, const QString &help)
{
    QLineEdit *field = new QLineEdit(parent);
    connect(field, &QLineEdit::textChanged, this, &MainWindow::showUnsavedState);
    if (help.isEmpty())
    {
        form->addRow(label, field);
        return field;
    }

    // The help sits right under its field rather than a whole form row away.
    QVBoxLayout *column = new QVBoxLayout;
    column->setSpacing(2);
    column->addWidget(field);
    column->addWidget(helpLabel(help, parent));
    form->addRow(label, column);
    return field;
}

QWidget *MainWindow::buildGeneralPage()
{
    QVBoxLayout *content = nullptr;
    QWidget *page = newPage(QStringLiteral("General"),
                            QStringLiteral("The game's identity. Every platform's package is labelled with it."),
                            &content);
    QWidget *parent = content->parentWidget();

    QFormLayout *form = new QFormLayout;
    content->addLayout(form);

    m_name = addField(form, parent, QStringLiteral("Name"));
    m_organization = addField(form, parent, QStringLiteral("Organization"),
                              QStringLiteral("Shown as the author on the Switch and the 3DS."));
    m_version = addField(form, parent, QStringLiteral("Version"),
                         QStringLiteral("Two digits, a dot and two digits (01.00), which is what the Vita requires."));
    m_windowTitle = addField(form, parent, QStringLiteral("Window title"),
                             QStringLiteral("The title of the game's window on PC."));
    return page;
}

QWidget *MainWindow::buildDisplayPage()
{
    QVBoxLayout *content = nullptr;
    QWidget *page = newPage(QStringLiteral("Display"),
                            QStringLiteral("How the game fills the screen. Every platform uses this display unless "
                                           "its own page gives it one."),
                            &content);
    QWidget *parent = content->parentWidget();

    m_sharedDisplayEditor = new DisplayEditor(parent);
    connect(m_sharedDisplayEditor, &DisplayEditor::edited, this, &MainWindow::sharedDisplayEdited);

    m_displayOverrides = new QLabel(parent);
    m_displayOverrides->setWordWrap(true);

    content->addWidget(m_sharedDisplayEditor);
    content->addSpacing(m_displayOverrides->fontMetrics().lineSpacing());
    content->addWidget(m_displayOverrides);
    return page;
}

QWidget *MainWindow::buildIconsPage()
{
    QVBoxLayout *content = nullptr;
    QWidget *page = newPage(QStringLiteral("Icons"),
                            QStringLiteral("Each platform reads its icon at a size and in a format of its own, so a "
                                           "project keeps one file per platform, all written from one image."),
                            &content);
    QWidget *parent = content->parentWidget();

    QGridLayout *grid = new QGridLayout;
    for (const Platform platform : s_platforms)
    {
        const int column = indexOf(platform);

        QLabel *preview = new QLabel(parent);
        preview->setFixedSize(s_iconPreviewSize, s_iconPreviewSize);
        preview->setAlignment(Qt::AlignCenter);
        preview->setFrameShape(QFrame::StyledPanel);
        m_iconPreviews[column] = preview;

        QLabel *path = helpLabel(Icon::path(platform), parent);

        grid->setColumnMinimumWidth(column, s_iconColumnWidth);
        grid->addWidget(new QLabel(platformName(platform), parent), 0, column);
        grid->addWidget(preview, 1, column);
        grid->addWidget(path, 2, column);
    }
    grid->setColumnStretch(s_platformCount, 1);

    QPushButton *change = new QPushButton(QStringLiteral("Change Icons..."), parent);
    connect(change, &QPushButton::clicked, this, &MainWindow::chooseIcon);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(change);
    buttons->addStretch();

    content->addLayout(grid);
    content->addLayout(buttons);
    return page;
}

QWidget *MainWindow::buildPlatformPage(Platform platform)
{
    QString description;
    switch (platform)
    {
    case Platform::Vita:
        description = QStringLiteral("What a PlayStation Vita package declares beyond the game's identity.");
        break;
    case Platform::Switch:
        description = QStringLiteral("A Switch .nro is labelled with the name, organization and version on the "
                                     "General page.");
        break;
    case Platform::Nintendo3ds:
        description = QStringLiteral("A 3DS .3dsx is labelled with the name, organization and version on the "
                                     "General page.");
        break;
    default:
        description = QStringLiteral("The game as a desktop executable.");
        break;
    }

    QVBoxLayout *content = nullptr;
    QWidget *page = newPage(platformName(platform), description, &content);
    QWidget *parent = content->parentWidget();

    if (platform == Platform::Vita)
    {
        QFormLayout *form = new QFormLayout;
        m_vitaTitleId = addField(form, parent, QStringLiteral("Title ID"),
                                 QStringLiteral("Four capital letters and five digits (FRIE00001). The console tells "
                                                "installed games apart by it, so no two may share one."));
        content->addLayout(form);
    }

    if (platform == Platform::Vita)
    {
        content->addWidget(buildLiveArea(parent));
    }

    content->addWidget(buildPlatformDisplay(platform, parent));
    return page;
}

QWidget *MainWindow::buildLiveArea(QWidget *parent)
{
    QGroupBox *group = new QGroupBox(QStringLiteral("LiveArea"), parent);

    QHBoxLayout *images = new QHBoxLayout;
    for (const LiveArea::Image image : {LiveArea::Image::Background, LiveArea::Image::Startup})
    {
        ImageFileEditor *editor = new ImageFileEditor(
            group, LiveArea::title(image), LiveArea::description(image), LiveArea::path(image), LiveArea::size(image),
            [image](const QImage &source, QStringList *notes) { return LiveArea::render(image, source, notes); });
        connect(editor, &ImageFileEditor::changed, this, &MainWindow::showUnsavedState);
        m_imageEditors << editor;
        images->addWidget(editor);
    }
    images->addStretch();

    m_liveAreaTemplate = new LiveAreaTemplateEditor(group);
    connect(m_liveAreaTemplate, &LiveAreaTemplateEditor::changed, this, &MainWindow::showUnsavedState);

    QVBoxLayout *layout = new QVBoxLayout(group);
    layout->addWidget(helpLabel(QStringLiteral("The game's page on the Vita's home screen. Images are cut to shape, "
                                               "resized and reduced to 256 colours, which is what the console reads."),
                                group));
    layout->addLayout(images);
    layout->addSpacing(layout->spacing());
    layout->addWidget(m_liveAreaTemplate);
    return group;
}

QWidget *MainWindow::buildPlatformDisplay(Platform platform, QWidget *parent)
{
    QGroupBox *group = new QGroupBox(QStringLiteral("Display"), parent);

    PlatformDisplay &display = m_platformDisplayEditors[indexOf(platform)];
    display.isOwn = new QCheckBox(QStringLiteral("Use a display of its own instead of the shared one"), group);
    display.editor = new DisplayEditor(group);

    QVBoxLayout *layout = new QVBoxLayout(group);
    layout->addWidget(display.isOwn);
    layout->addWidget(display.editor);

    connect(display.isOwn, &QCheckBox::toggled, this,
            [this, platform](bool isOwn) { ownDisplayToggled(platform, isOwn); });
    connect(display.editor, &DisplayEditor::edited, this, [this, platform] { platformDisplayEdited(platform); });
    return group;
}

void MainWindow::showHome()
{
    if (m_pages->currentWidget() == m_projectView && !confirmLeavingProject())
    {
        return;
    }

    m_project = Project();
    m_buildBar->setProjectDirectory(QString());
    m_build->setEnabled(false);
    m_save->setEnabled(false);
    m_icon->setEnabled(false);
    m_close->setEnabled(false);

    m_home->refresh();
    m_pages->setCurrentWidget(m_home);
    setWindowTitle(s_applicationTitle);
    setWindowModified(false);
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
        showIcons();
        QMessageBox::information(this, s_applicationTitle, dialog.report());
    }
}

void MainWindow::showIcons()
{
    const QDir directory(m_project.directory());
    for (const Platform platform : s_platforms)
    {
        QLabel *preview = m_iconPreviews[indexOf(platform)];
        const QImage icon(directory.filePath(Icon::path(platform)));
        if (icon.isNull())
        {
            preview->setPixmap(QPixmap());
            preview->setText(QStringLiteral("none"));
            continue;
        }

        // A small console icon is shown at its own size rather than blurred up.
        const QPixmap pixmap = QPixmap::fromImage(icon);
        preview->setPixmap(pixmap.width() > s_iconPreviewSize
                               ? pixmap.scaled(s_iconPreviewSize, s_iconPreviewSize, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation)
                               : pixmap);
        preview->setToolTip(QStringLiteral("%1x%2").arg(icon.width()).arg(icon.height()));
    }
}

void MainWindow::showDisplays()
{
    m_sharedDisplayEditor->setDisplay(m_sharedDisplay);

    QStringList overridden;
    for (const Platform platform : s_platforms)
    {
        const std::optional<DisplaySettings> &own = m_platformDisplays[indexOf(platform)];
        const PlatformDisplay &display = m_platformDisplayEditors[indexOf(platform)];

        const QSignalBlocker blocker(display.isOwn);
        display.isOwn->setChecked(own.has_value());
        display.editor->setDisplay(own ? *own : m_sharedDisplay);
        display.editor->setEnabled(own.has_value());

        if (own)
        {
            overridden << platformName(platform);
        }
    }

    if (overridden.isEmpty())
    {
        m_displayOverrides->setText(QStringLiteral("Every platform uses this display."));
    }
    else if (overridden.size() == s_platformCount)
    {
        m_displayOverrides->setText(QStringLiteral("Every platform has a display of its own, so this one is not "
                                                   "used. Each platform's page says what it uses instead."));
    }
    else
    {
        m_displayOverrides->setText(QStringLiteral("Not used by %1, which have a display of their own.")
                                        .arg(overridden.join(QStringLiteral(", "))));
    }

    showUnsavedState();
}

void MainWindow::sharedDisplayEdited()
{
    m_sharedDisplay = m_sharedDisplayEditor->display();
    showDisplays();
}

void MainWindow::platformDisplayEdited(Platform platform)
{
    m_platformDisplays[indexOf(platform)] = m_platformDisplayEditors[indexOf(platform)].editor->display();
    showUnsavedState();
}

// A platform's own display starts as a copy of the shared one.
void MainWindow::ownDisplayToggled(Platform platform, bool isOwn)
{
    m_platformDisplays[indexOf(platform)] = isOwn ? std::optional<DisplaySettings>(m_sharedDisplay) : std::nullopt;
    showDisplays();
}

bool MainWindow::hasUnsavedDisplay() const
{
    if (m_sharedDisplay != m_project.display())
    {
        return true;
    }

    for (const Platform platform : s_platforms)
    {
        if (m_platformDisplays[indexOf(platform)] != m_project.display(platform))
        {
            return true;
        }
    }
    return false;
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
    for (const Platform platform : s_platforms)
    {
        m_platformDisplays[indexOf(platform)] = m_project.display(platform);
    }
    showDisplays();

    showIcons();
    for (ImageFileEditor *editor : std::as_const(m_imageEditors))
    {
        editor->setProjectDirectory(m_project.directory());
    }
    m_liveAreaTemplate->setProjectDirectory(m_project.directory());
    m_buildBar->setProjectDirectory(m_project.directory());

    m_pages->setCurrentWidget(m_projectView);
    m_build->setEnabled(true);
    m_save->setEnabled(true);
    m_icon->setEnabled(true);
    m_close->setEnabled(true);
    setWindowTitle(QStringLiteral("%1[*] - %2").arg(m_project.name(), s_applicationTitle));
    showUnsavedState();
    statusBar()->showMessage(QDir::toNativeSeparators(m_project.filePath()));
}

bool MainWindow::hasUnsavedEdits() const
{
    return m_name->text() != m_project.name()
           || m_organization->text() != m_project.organization()
           || m_version->text() != m_project.version()
           || m_windowTitle->text() != m_project.windowTitle()
           || m_vitaTitleId->text() != m_project.vitaTitleId()
           || hasUnsavedDisplay()
           || hasUnsavedFiles();
}

bool MainWindow::hasUnsavedFiles() const
{
    if (m_liveAreaTemplate->hasPending())
    {
        return true;
    }

    for (const ImageFileEditor *editor : m_imageEditors)
    {
        if (editor->hasPending())
        {
            return true;
        }
    }
    return false;
}

// Written first, so a failure to write them leaves project.fried as it was.
bool MainWindow::writeFiles()
{
    QList<ImageConversion::File> files;
    for (const ImageFileEditor *editor : std::as_const(m_imageEditors))
    {
        files << editor->pending();
    }

    QString error;
    if (!ImageConversion::write(m_project.directory(), files, &error))
    {
        QMessageBox::warning(this, QStringLiteral("Save"), error);
        return false;
    }

    for (ImageFileEditor *editor : std::as_const(m_imageEditors))
    {
        editor->pendingWritten();
    }

    if (!m_liveAreaTemplate->writePending(&error))
    {
        QMessageBox::warning(this, QStringLiteral("Save"), error);
        return false;
    }
    return true;
}

void MainWindow::showUnsavedState()
{
    const bool isUnsaved = !m_project.directory().isEmpty() && hasUnsavedEdits();
    setWindowModified(isUnsaved);
    m_saveButton->setEnabled(isUnsaved);
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

    m_buildBar->remember();

    const Platform platform = m_buildBar->platform();
    const Build::Type type = m_buildBar->type();

    QString error;
    const QList<QStringList> commands = Build::commands(platform, type, m_buildBar->directory(), &error);
    if (commands.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("Build"), error);
        return;
    }

    CommandDialog build(this, QStringLiteral("Build %1 (%2)").arg(platformName(platform), Build::name(type)),
                        m_project.directory(), commands);
    build.setActivity(QStringLiteral("Compiling"));
    build.offerToOpen(QStringLiteral("Open Build Folder"), QDir(m_project.directory()).filePath(m_buildBar->directory()));
    build.exec();

    statusBar()->showMessage(build.succeeded()
                                 ? QStringLiteral("Built %1 for %2").arg(m_project.name(), platformName(platform))
                                 : QStringLiteral("Build for %1 did not finish").arg(platformName(platform)));
}

QString MainWindow::firstProblem(QLineEdit **field) const
{
    const QList<std::tuple<QLineEdit *, QString, QString>> problems = {
        {m_name, QStringLiteral("Name"), Project::checkIdentity(m_name->text())},
        {m_organization, QStringLiteral("Organization"), Project::checkIdentity(m_organization->text())},
        {m_version, QStringLiteral("Version"), Project::checkVersion(m_version->text())},
        {m_vitaTitleId, QStringLiteral("Vita title ID"), Project::checkVitaTitleId(m_vitaTitleId->text())},
    };

    for (const auto &[edit, label, problem] : problems)
    {
        if (problem.isEmpty())
        {
            continue;
        }

        *field = edit;
        return QStringLiteral("%1 %2").arg(label, problem);
    }

    return QString();
}

bool MainWindow::saveProject()
{
    QLineEdit *invalid = nullptr;
    const QString problem = firstProblem(&invalid);
    if (!problem.isEmpty())
    {
        showSectionHolding(invalid);
        QMessageBox::warning(this, QStringLiteral("Save"), problem);
        invalid->setFocus();
        invalid->selectAll();
        return false;
    }

    if (!writeFiles())
    {
        return false;
    }

    m_project.setName(m_name->text());
    m_project.setOrganization(m_organization->text());
    m_project.setVersion(m_version->text());
    m_project.setWindowTitle(m_windowTitle->text());
    m_project.setVitaTitleId(m_vitaTitleId->text());
    m_project.setDisplay(m_sharedDisplay);
    for (const Platform platform : s_platforms)
    {
        m_project.setDisplay(platform, m_platformDisplays[indexOf(platform)]);
    }

    QString error;
    if (!m_project.save(&error))
    {
        QMessageBox::warning(this, QStringLiteral("Save"), error);
        return false;
    }

    setWindowTitle(QStringLiteral("%1[*] - %2").arg(m_project.name(), s_applicationTitle));
    showUnsavedState();
    statusBar()->showMessage(QStringLiteral("Saved %1").arg(QDir::toNativeSeparators(m_project.filePath())));
    return true;
}
