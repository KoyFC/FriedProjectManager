#pragma once

#include "project.h"

#include <QList>
#include <QMainWindow>

#include <array>
#include <optional>

class AssetPanel;
class BuildBar;
class ToolchainPanel;
class DisplayEditor;
class HomePage;
class ImageFileEditor;
class LiveAreaTemplateEditor;

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
    // Text only one platform's package shows, which falls back to the game's identity while empty.
    struct PackagingField
    {
        QString section;
        QString key;
        QLineEdit *edit = nullptr;
    };

    void buildPages();
    QWidget *buildProjectView();
    QWidget *buildGeneralPage();
    QWidget *buildDisplayPage();
    QWidget *buildIconsPage();
    QWidget *buildEnginePage();
    QWidget *buildPlatformPage(Platform platform);
    QWidget *buildPlatformIcon(Platform platform, QWidget *parent);
    QWidget *buildPackagingText(Platform platform, QWidget *parent);
    void showPackagingPlaceholders();
    QWidget *buildLiveArea(QWidget *parent);
    QWidget *buildBootScreen(QWidget *parent);
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
    void openInEditor();
    void refreshToolchains();
    void refreshAssetPanel(Platform platform);
    bool downloadCompiler();
    bool downloadEmscripten();
    void showStorage();
    void chooseIcon();
    void showIcons();
    void showIconIn(QLabel *preview, Platform platform);
    void changePlatformIcon(Platform platform);
    void showEngineVersion();
    void changeEngineVersion();
    bool confirmUnsavedEdits();
    bool hasUnsavedEdits() const;
    void showUnsavedState();
    void chooseProject();
    QString nearbyLocation() const;
    void showProject();
    void showDisplays();
    void sharedDisplayEdited();
    bool hasUnsavedDisplay() const;
    bool hasUnsavedPackagingText() const;
    bool hasUnsavedFiles() const;
    bool writeFiles();
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QStackedWidget *m_pages = nullptr;
    HomePage *m_home = nullptr;
    QWidget *m_projectView = nullptr;
    QList<QListWidget *> m_sectionLists;
    QStackedWidget *m_sections = nullptr;
    BuildBar *m_buildBar = nullptr;
    std::array<ToolchainPanel *, s_platformCount> m_toolchainPanels = {};
    std::array<AssetPanel *, s_platformCount> m_assetPanels = {};
    QPushButton *m_saveButton = nullptr;

    QAction *m_build = nullptr;
    QAction *m_save = nullptr;
    QAction *m_close = nullptr;
    QAction *m_icon = nullptr;
    QAction *m_editor = nullptr;

    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
    std::array<QLabel *, s_platformCount> m_iconPreviews = {};
    std::array<QLabel *, s_platformCount> m_platformIconPreviews = {};
    QList<PackagingField> m_packagingFields;
    QLabel *m_engineVersion = nullptr;
    QPushButton *m_changeEngineVersion = nullptr;
    DisplayEditor *m_sharedDisplayEditor = nullptr;
    QList<ImageFileEditor *> m_imageEditors;
    LiveAreaTemplateEditor *m_liveAreaTemplate = nullptr;

    // The form's edits to every platform's display, until they are saved.
    DisplaySettings m_sharedDisplay;
};
