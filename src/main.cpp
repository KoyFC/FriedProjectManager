#include "main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName("Fried Project Manager");

    MainWindow window;
    window.show();

    return application.exec();
}
