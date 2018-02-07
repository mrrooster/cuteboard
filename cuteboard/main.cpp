#include <QApplication>
#include "cuteboard.h"

int main(int argc, char *argv[])
{
    // Must be done before cuteboard is created
    QCoreApplication::setOrganizationName("cuteboard");
    QCoreApplication::setOrganizationDomain("cuteboard.ohmyno.co.uk");
    QCoreApplication::setApplicationName("Cuteboard");

    QApplication a(argc, argv);
    Cuteboard c;
    c.start();

    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
