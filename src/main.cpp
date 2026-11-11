#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("Fried Project Manager");

    // QSettings needs both names to pick a file to remember dismissed notices in.
    application.setOrganizationName("Fried Engine");

    MainWindow window;
    window.show();

    // A project's folder on the command line opens it straight away.
    const QStringList arguments = application.arguments();
    if (arguments.size() > 1)
    {
        window.openProject(arguments.at(1));
    }

    return application.exec();
}
