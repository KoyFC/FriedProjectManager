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

    return application.exec();
}
