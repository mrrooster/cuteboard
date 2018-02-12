#include <QApplication>
#include <QSysInfo>
#include "cuteboard.h"

int main(int argc, char *argv[])
{
    // Must be done before cuteboard is created
    QCoreApplication::setOrganizationName("cuteboard");
    QCoreApplication::setOrganizationDomain("cuteboard.ohmyno.co.uk");
    QCoreApplication::setApplicationName("Cuteboard");
    QCoreApplication::setApplicationVersion(QString("cuteboard %1 build date: %2 %3 %4")
                                            .arg("180211")
                                            .arg(__DATE__)
                                            .arg(QSysInfo::prettyProductName())
                                            .arg(QSysInfo::buildCpuArchitecture())
                                            );

    QApplication a(argc, argv);
    Cuteboard c;
    c.start();

    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
