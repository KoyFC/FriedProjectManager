#pragma once

#include "project.h"

#include <QList>
#include <QMainWindow>

#include <array>
#include <optional>

class BuildBar;
class DisplayEditor;
class HomePage;

class QAction;
class QCheckBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QVBoxLayout;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    // Both show their own error and leave the open project as it was.
    bool openProject(const QString &directory);
    bool saveProject();

private:
    struct PlatformDisplay
    {
        QCheckBox *isOwn = nullptr;
        DisplayEditor *editor = nullptr;
    };

    void buildPages();
    QWidget *buildProjectView();
    QWidget *buildGeneralPage();
    QWidget *buildDisplayPage();
    QWidget *buildIconsPage();
    QWidget *buildPlatformPage(Platform platform);
    QWidget *buildPlatformDisplay(Platform platform, QWidget *parent);
    QWidget *newPage(const QString &title, const QString &description, QVBoxLayout **content);
    QListWidget *newSectionList(QWidget *parent);
    void addSection(QListWidget *list, const QString &name, QWidget *page);
    void showSectionHolding(QWidget *widget);
    QLineEdit *addField(QFormLayout *form, QWidget *parent, const QString &label, const QString &help = QString());

    void newProject();
    void showHome();
    bool confirmLeavingProject();
    bool confirmGitNotice();
    void buildProject();
    void chooseIcon();
    void showIcons();
    bool confirmUnsavedEdits();
    bool hasUnsavedEdits() const;
    void showUnsavedState();
    void chooseProject();
    QString nearbyLocation() const;
    void showProject();
    void showDisplays();
    void sharedDisplayEdited();
    void platformDisplayEdited(Platform platform);
    void ownDisplayToggled(Platform platform, bool isOwn);
    bool hasUnsavedDisplay() const;
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QStackedWidget *m_pages = nullptr;
    HomePage *m_home = nullptr;
    QWidget *m_projectView = nullptr;
    QList<QListWidget *> m_sectionLists;
    QStackedWidget *m_sections = nullptr;
    BuildBar *m_buildBar = nullptr;
    QPushButton *m_saveButton = nullptr;

    QAction *m_build = nullptr;
    QAction *m_save = nullptr;
    QAction *m_close = nullptr;
    QAction *m_icon = nullptr;

    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
    std::array<QLabel *, s_platformCount> m_iconPreviews = {};
    DisplayEditor *m_sharedDisplayEditor = nullptr;
    QLabel *m_displayOverrides = nullptr;
    std::array<PlatformDisplay, s_platformCount> m_platformDisplayEditors = {};

    // The form's edits to every platform's display, until they are saved.
    DisplaySettings m_sharedDisplay;
    std::array<std::optional<DisplaySettings>, s_platformCount> m_platformDisplays;
};
