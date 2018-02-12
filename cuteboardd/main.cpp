#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "cuteboardserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    CuteboardServer server;
    QCommandLineParser parser;
    QCoreApplication::setApplicationName("cuteboardd");
    QCoreApplication::setApplicationVersion(QString("%1 build date: %2 (%3) %4")
                                            .arg("180211")
                                            .arg(__DATE__)
                                            .arg(QSysInfo::prettyProductName())
                                            .arg(QSysInfo::buildCpuArchitecture())
                                            );

    parser.setApplicationDescription("A server for the cuteboard protocol.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption({{"a","address"},QCoreApplication::translate("main","Set the address to listen on."),QCoreApplication::translate("main","address"),""});
    parser.addOption({{"p","port"},QCoreApplication::translate("main","Set the port to listen on."),QCoreApplication::translate("main","port"),"19780"});

    parser.process(a);

    QString address = parser.value(QCoreApplication::translate("main","address"));
    QString port    = parser.value(QCoreApplication::translate("main","port"));

    if (!address.isEmpty()) {
        server.setListenAddress(address);
    }
    if (!port.isEmpty()) {
        server.setListenPort(port.toUShort());
    }

    server.commence();

    return a.exec();
}
