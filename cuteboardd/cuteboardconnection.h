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
#ifndef CUTEBOARDCONNECTION_H
#define CUTEBOARDCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QUuid>
#include <QTimer>
#include "userstate.h"

class CuteboardConnection : public QObject
{
    Q_OBJECT
public:
    explicit CuteboardConnection(QTcpSocket *socket, QObject *parent = nullptr);

    void close();
signals:
    void clipboardData(CuteboardConnection *source,QString data);
    void connectionClosed(CuteboardConnection *connection);

public slots:

private:
    enum ConnectionState {
        Error,New,PreAuth,AuthChallenge,Connected
    };

    QTcpSocket *s;
    QTimer watchdog;
    ConnectionState state;
    QString user;
    QString client;
    QUuid connectionId;
    QUuid challenge;
    QByteArray readBuffer;
    int maxBufferSize; // Disconnect if exceeded
    static QMap<QString,UserState*> userMap;

    void setState(ConnectionState newState);
    void setError(QString message);
    void write(QString message);
    void handleLogin();
    void handleChallengeResponse(QString response);
    void handleClipboardCommand(QString data);
    QPair<QString,QString> readLine();
private slots:
    void handleWatchdogTimeout();
    void handleReadyToRead();
    void handleClipboardData(CuteboardConnection *source,QString data);
};

#endif // CUTEBOARDCONNECTION_H
