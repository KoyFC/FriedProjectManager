#include "editor_launcher.h"

#include "command_dialog.h"
#include "editor_install.h"
#include "editor_version_dialog.h"

#include <QCoreApplication>
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

QString EditorLauncher::localExecutable()
{
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
    return QString();
}

QString EditorLauncher::findExecutable()
{
    const QString local = localExecutable();
    if (!local.isEmpty())
    {
        return local;
    }

    const QStringList installed = EditorInstall::installedVersions();
    return installed.isEmpty() ? QString() : EditorInstall::executable(installed.first());
}

bool EditorLauncher::install(QWidget *parent)
{
    EditorVersionDialog choose(parent);
    if (choose.exec() != QDialog::Accepted || choose.chosenVersion().isEmpty())
    {
        return false;
    }
    const QString version = choose.chosenVersion();

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
            parent, QStringLiteral("Open in Editor"), QStringLiteral("Fried Editor is not installed. Choose a version to download now?"));
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
