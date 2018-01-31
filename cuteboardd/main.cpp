#include <QCoreApplication>
#include "cuteboardserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    CuteboardServer server;

    server.commence();

    return a.exec();
}
