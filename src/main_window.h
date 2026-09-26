#pragma once

#include "project.h"

#include <QMainWindow>

class QAction;
class QComboBox;
class QFormLayout;
class QLineEdit;
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
    void buildForm();
    QWidget *buildPlatformRow();
    QWidget *buildDirectoryRow();
    QLineEdit *addField(const QString &label);

    void newProject();
    bool confirmGitNotice();
    void buildProject();
    void chooseIcon();
    void chooseBuildDirectory();
    void showBuildDirectory();
    void rememberBuildDirectory();
    bool confirmUnsavedEdits();
    bool hasUnsavedEdits() const;
    void chooseProject();
    QString nearbyLocation() const;
    void showProject();
    void showPlatformFields();
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QWidget *m_form = nullptr;
    QFormLayout *m_fields = nullptr;
    QComboBox *m_platform = nullptr;
    QAction *m_build = nullptr;
    QAction *m_icon = nullptr;
    QLineEdit *m_buildDirectory = nullptr;
    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
};
