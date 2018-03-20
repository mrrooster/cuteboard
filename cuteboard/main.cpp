/**
  * (c) 2018 Ian Clark
  *
  * This file is part of cuteboard
  *
  * Cuteboard is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * Cuteboard is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * You should have received a copy of the GNU General Public License
  * along with Cuteboard.  If not, see <http://www.gnu.org/licenses/>.
  */
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
