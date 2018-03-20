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
#ifndef CUTEBOARDSERVER_H
#define CUTEBOARDSERVER_H

#include <QObject>
#include <QTcpServer>
#include "cuteboardconnection.h"

class CuteboardServer : public QObject
{
    Q_OBJECT
public:
    explicit CuteboardServer(QObject *parent = nullptr);

    void setListenAddress(QString address);
    void setListenPort(quint16 port);
signals:

public slots:
    void commence();

private:
    QTcpServer *server;
    QVector<CuteboardConnection*> connections;
    QString hostAddress;
    quint16 port;

private slots:
    void handleIncommingConnection();
    void handleConnectionClosed(CuteboardConnection* connection);
};

#endif // CUTEBOARDSERVER_H
