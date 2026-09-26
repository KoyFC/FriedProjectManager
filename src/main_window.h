#pragma once

#include "project.h"

#include <QMainWindow>

class QFormLayout;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

    // Reports the failure and keeps the current project.
    bool openProject(const QString &directory);

private:
    void buildForm();
    QLineEdit *addField(QFormLayout *layout, const QString &label);

    void chooseProject();
    void showProject();

    Project m_project;

    QWidget *m_form = nullptr;
    QLineEdit *m_name = nullptr;
    QLineEdit *m_organization = nullptr;
    QLineEdit *m_version = nullptr;
    QLineEdit *m_windowTitle = nullptr;
    QLineEdit *m_vitaTitleId = nullptr;
};
