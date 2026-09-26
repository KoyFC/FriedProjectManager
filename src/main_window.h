#pragma once

#include "project.h"

#include <QMainWindow>

class QComboBox;
class QFormLayout;
class QLineEdit;

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
    QLineEdit *addField(const QString &label);

    void chooseProject();
    void showProject();
    void showPlatformFields();
    QString firstProblem(QLineEdit **field) const;

    Project m_project;

    QWidget *m_form = nullptr;
    QFormLayout *m_fields = nullptr;
    QComboBox *m_platform = nullptr;
    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
};
