#pragma once

#include "build.h"
#include "project.h"

#include <QMainWindow>

#include <array>
#include <optional>

class HomePage;

class QAction;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
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
    void buildPages();
    void buildForm();
    QWidget *buildPlatformRow();
    QWidget *buildIconRow();
    QWidget *buildTypeRow();
    QWidget *buildDirectoryRow();
    QWidget *buildDisplayRow();
    QLineEdit *addField(const QString &label);

    void newProject();
    void showHome();
    bool confirmLeavingProject();
    bool confirmGitNotice();
    void buildProject();
    void chooseIcon();
    void showIcon();
    void chooseBuildDirectory();
    void showBuildType();
    void rememberBuildType();
    Build::Type selectedBuildType() const;
    void resetBuildDirectory();
    void showBuildDirectory();
    void showBuildDirectory(const QString &directory);
    void buildDirectoryChanged();
    QString chosenBuildDirectory() const;
    QString vitaSpaceProblem() const;
    void rememberBuildDirectory();
    bool confirmUnsavedEdits();
    bool hasUnsavedEdits() const;
    void chooseProject();
    Platform selectedPlatform() const;
    QString nearbyLocation() const;
    void showProject();
    void showPlatformFields();
    void showDisplay();
    void displayEdited();
    void ownDisplayToggled(bool isOwn);
    DisplaySettings displayInForm() const;
    std::optional<DisplaySettings> &platformDisplay();
    bool hasUnsavedDisplay() const;
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QStackedWidget *m_pages = nullptr;
    HomePage *m_home = nullptr;
    QWidget *m_form = nullptr;
    QFormLayout *m_fields = nullptr;
    QComboBox *m_platform = nullptr;
    QComboBox *m_buildType = nullptr;
    QAction *m_build = nullptr;
    QAction *m_close = nullptr;
    QAction *m_icon = nullptr;
    QLabel *m_iconPreview = nullptr;
    QLineEdit *m_buildDirectory = nullptr;
    QPushButton *m_buildReset = nullptr;
    QLabel *m_buildWarning = nullptr;
    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
    QComboBox *m_displayMode = nullptr;
    QSpinBox *m_displayWidth = nullptr;
    QSpinBox *m_displayHeight = nullptr;
    QComboBox *m_displayFilter = nullptr;
    QCheckBox *m_ownDisplay = nullptr;

    // The form's edits to every platform's display, until they are saved.
    DisplaySettings m_sharedDisplay;
    std::array<std::optional<DisplaySettings>, s_platformCount> m_platformDisplays;
    bool m_isShowingDisplay = false;
};
