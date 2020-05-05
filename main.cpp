#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Vernocte laboratories");
    QCoreApplication::setOrganizationDomain("vernocte.org");
    QCoreApplication::setApplicationName("Serial Port Commander");

    MainWindow w;
    w.show();

    return a.exec();
}
