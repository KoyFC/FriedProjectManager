#pragma once

#include "build.h"

#include <QWidget>

class QAction;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

// What to build and where, remembered per project and per target.
class BuildBar : public QWidget
{
    Q_OBJECT

public:
    BuildBar(QWidget *parent, QAction *build);

    // An empty directory means no project is open, and nothing is remembered.
    void setProjectDirectory(const QString &directory);

    BuildTarget target() const;
    Build::Type type() const;
    bool logToFile() const;

    // Relative to the project, unless it was given as an absolute path.
    QString directory() const;

    void remember();

private:
    void showRemembered();
    void showDirectory(const QString &directory);
    void directoryChanged();
    void chooseDirectory();
    void resetDirectory();
    void rememberType();
    void rememberLogToFile();
    void rememberDirectory();
    QString vitaSpaceProblem() const;

    QString m_projectDirectory;

    QComboBox *m_target = nullptr;
    QComboBox *m_type = nullptr;
    QCheckBox *m_logToFile = nullptr;
    QLineEdit *m_directory = nullptr;
    QPushButton *m_reset = nullptr;
    QLabel *m_warning = nullptr;
};
