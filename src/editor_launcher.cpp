#include "editor_launcher.h"

#include "command_dialog.h"
#include "editor_install.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

namespace
{
    bool isRunnable(const QString &path)
    {
        const QFileInfo info(path);
        return info.isFile() && info.isExecutable();
    }
}

QString EditorLauncher::overrideExecutable()
{
    const QString fromEnvironment = qEnvironmentVariable("FRIED_EDITOR");
    return isRunnable(fromEnvironment) ? fromEnvironment : QString();
}

QString EditorLauncher::findExecutable()
{
    const QString override = overrideExecutable();
    if (!override.isEmpty())
    {
        return override;
    }

    const QStringList installed = EditorInstall::installedVersions();
    return installed.isEmpty() ? QString() : EditorInstall::executable(installed.first());
}

bool EditorLauncher::install(QWidget *parent)
{
    const QString version = EditorInstall::latestVersion();
    const QMessageBox::StandardButton answer = QMessageBox::question(
        parent, QStringLiteral("Fried Editor"),
        QStringLiteral("The Project Manager can download Fried Editor %1 from GitHub, a few MB, into %2 and open "
                       "your projects with it. Download it now?")
            .arg(version, QDir::toNativeSeparators(EditorInstall::versionsDirectory())));
    if (answer != QMessageBox::Yes)
    {
        return false;
    }

    // The commands are listed first, since listing them clears what an earlier attempt left and makes the folder they run in.
    const QList<QStringList> commands = EditorInstall::beginInstall(version);
    CommandDialog download(parent, QStringLiteral("Download Fried Editor %1").arg(version),
                           EditorInstall::unpackDirectory(version), commands);
    download.setActivity(QStringLiteral("Downloading"));
    download.exec();

    QString error;
    if (!EditorInstall::finishInstall(version, download.succeeded(), &error))
    {
        // A download that failed already said why in its own window.
        if (download.succeeded())
        {
            QMessageBox::warning(parent, QStringLiteral("Fried Editor"), error);
        }
        return false;
    }

    return true;
}

bool EditorLauncher::launch(const QString &projectDirectory, QWidget *parent)
{
    QString executable = findExecutable();
    if (executable.isEmpty())
    {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            parent, QStringLiteral("Open in Editor"), QStringLiteral("Fried Editor is not installed. Download it now?"));
        if (answer != QMessageBox::Yes || !install(parent))
        {
            return false;
        }
        executable = findExecutable();
    }

    if (!QProcess::startDetached(executable, {projectDirectory}))
    {
        QMessageBox::warning(parent, QStringLiteral("Open in Editor"), QStringLiteral("Could not start %1.").arg(executable));
        return false;
    }
    return true;
}
