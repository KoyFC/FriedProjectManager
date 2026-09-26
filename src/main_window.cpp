#include "main_window.h"

#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

namespace
{
    const QString s_applicationTitle = QStringLiteral("Fried Project Manager");
}

MainWindow::MainWindow()
{
    setWindowTitle(s_applicationTitle);
    resize(900, 600);

    QMenu *fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("&Open Project..."), QKeySequence::Open, this, &MainWindow::chooseProject);

    buildForm();
    statusBar()->showMessage(QStringLiteral("No project open"));
}

void MainWindow::buildForm()
{
    m_form = new QWidget(this);

    QVBoxLayout *column = new QVBoxLayout(m_form);
    QFormLayout *fields = new QFormLayout();
    column->addLayout(fields);
    column->addStretch();

    m_name = addField(fields, QStringLiteral("Name"));
    m_organization = addField(fields, QStringLiteral("Organization"));
    m_version = addField(fields, QStringLiteral("Version"));
    m_windowTitle = addField(fields, QStringLiteral("Window title"));
    m_vitaTitleId = addField(fields, QStringLiteral("Vita title ID"));

    m_form->setEnabled(false);
    setCentralWidget(m_form);
}

QLineEdit *MainWindow::addField(QFormLayout *layout, const QString &label)
{
    QLineEdit *field = new QLineEdit(m_form);
    field->setReadOnly(true);
    layout->addRow(label, field);
    return field;
}

void MainWindow::chooseProject()
{
    const QString startAt = m_project.directory().isEmpty()
                                ? QDir::homePath()
                                : QDir(m_project.directory()).filePath(QStringLiteral(".."));

    const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Open Project"), startAt);
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

    m_form->setEnabled(true);
    setWindowTitle(QStringLiteral("%1 - %2").arg(m_project.name(), s_applicationTitle));
    statusBar()->showMessage(QDir::toNativeSeparators(m_project.filePath()));
}
