#include "editor_launcher.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>

namespace
{
    const QString s_editorKey = QStringLiteral("editorExecutable");

    bool isRunnable(const QString &path)
    {
        const QFileInfo info(path);
        return info.isFile() && info.isExecutable();
    }
}

QString EditorLauncher::findExecutable()
{
    const QString remembered = QSettings().value(s_editorKey).toString();
    if (isRunnable(remembered))
    {
        return remembered;
    }

    const QString fromEnvironment = qEnvironmentVariable("FRIED_EDITOR");
    if (isRunnable(fromEnvironment))
    {
        return fromEnvironment;
    }

    // The repositories are siblings, as the engine's and this one's are.
    const QDir beside(QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/../../FriedEditor/build/app")));
    for (const QString &name : {QStringLiteral("fried_editor"), QStringLiteral("fried_editor.exe")})
    {
        if (isRunnable(beside.filePath(name)))
        {
            return beside.filePath(name);
        }
    }

    const QDir home(QDir::homePath() + QStringLiteral("/Documentos/GitHub/FriedEditor/build/app"));
    return isRunnable(home.filePath(QStringLiteral("fried_editor"))) ? home.filePath(QStringLiteral("fried_editor")) : QString();
}

bool EditorLauncher::launch(const QString &projectDirectory, QWidget *parent)
{
    QString executable = findExecutable();
    if (executable.isEmpty())
    {
        executable = QFileDialog::getOpenFileName(parent, QStringLiteral("Find Fried Editor"), QDir::homePath());
        if (executable.isEmpty())
        {
            return false;
        }
        if (!isRunnable(executable))
        {
            QMessageBox::warning(parent, QStringLiteral("Open in Editor"), QStringLiteral("%1 is not an executable.").arg(executable));
            return false;
        }
        QSettings().setValue(s_editorKey, executable);
    }

    if (!QProcess::startDetached(executable, {projectDirectory}))
    {
        QMessageBox::warning(parent, QStringLiteral("Open in Editor"), QStringLiteral("Could not start %1.").arg(executable));
        return false;
    }
    return true;
}
