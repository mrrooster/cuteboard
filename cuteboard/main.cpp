#include <QApplication>
#include "cuteboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Cuteboard c;

    c.start();

    return a.exec();
}
