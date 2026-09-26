#pragma once

#include "platform.h"
#include "project.h"

#include <QMainWindow>

class HomePage;

class QAction;
class QComboBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
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
    QWidget *buildDirectoryRow();
    QLineEdit *addField(const QString &label);

    void newProject();
    void showHome();
    bool confirmLeavingProject();
    bool confirmGitNotice();
    void buildProject();
    void chooseIcon();
    void showIcon();
    void chooseBuildDirectory();
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
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QStackedWidget *m_pages = nullptr;
    HomePage *m_home = nullptr;
    QWidget *m_form = nullptr;
    QFormLayout *m_fields = nullptr;
    QComboBox *m_platform = nullptr;
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
};
