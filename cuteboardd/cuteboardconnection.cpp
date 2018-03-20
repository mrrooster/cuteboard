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
#include "cuteboardconnection.h"

#define DEBUG_CONNECTION
#ifdef DEBUG_CONNECTION
#include <QDebug>
#define D(a) qDebug() <<QString("[CuteboardConnection %1]").arg(this->connectionId.toString()).toUtf8().data() <<a
#else
#define D(a)
#endif

QMap<QString,UserState*> CuteboardConnection::userMap;

CuteboardConnection::CuteboardConnection(QTcpSocket *socket, QObject *parent) :
    QObject(parent),
    state(New),
    challenge(QUuid::createUuid()),
    connectionId(QUuid::createUuid()),
    user(""),
    client(""),
    maxBufferSize(1024*1024*80)
{
    D("Got incoming connection.");
    this->s = socket;

    connect(this->s,&QTcpSocket::readyRead,this,&CuteboardConnection::handleReadyToRead);
    connect(&this->watchdog,&QTimer::timeout,this,&CuteboardConnection::handleWatchdogTimeout);

    // Require auth within 10 sec
    this->watchdog.setSingleShot(true);
    this->watchdog.start(10000);

    if (s->isReadable()) {
        handleReadyToRead();
    }
}

void CuteboardConnection::close()
{
    this->s->close();
    emit connectionClosed(this);
}

void CuteboardConnection::setState(CuteboardConnection::ConnectionState newState)
{
    D("Changing state from"<<this->state<<"to:"<<newState);
    if (newState==Connected) {
        this->watchdog.start(600000);
    } else if (newState!=Error) {
        this->watchdog.start();
    }
    this->state = newState;
    // TODO: This
}

void CuteboardConnection::setError(QString message)
{
    D("Error on connection: "<<message);
    write(QString("Error: %1").arg(message).toUtf8());
    close();
}

void CuteboardConnection::write(QString message)
{
    this->s->write(QString("%1\r\n").arg(message).toUtf8());
}

void CuteboardConnection::handleLogin()
{
    if (this->user.isEmpty() || this->client.isEmpty()) {
        return;
    }
    setState(AuthChallenge);

    UserState *dets;
    if (this->userMap.contains(this->user)) {
        dets = this->userMap.value(user);
    } else {
        dets = new UserState();
        dets->challenge = this->challenge;
        this->userMap.insert(user,dets);
    }
    write(QString("Challenge: %1").arg(dets->challenge.toString()));
}

void CuteboardConnection::handleChallengeResponse(QString response)
{
    if (this->userMap.contains(this->user) && !response.isEmpty()) {
        UserState *dets = this->userMap.value(this->user);
        if (dets->response.isEmpty()) {
            D("New user, welcome!");
            dets->response = response;
        }
        if (dets->response==response) {
            D("User authenticated.");
            connect(this,&CuteboardConnection::clipboardData,dets,&UserState::clipboardData);
            connect(dets,&UserState::clipboardData,this,&CuteboardConnection::handleClipboardData);
            write("Error: 0/0 OK");
            setState(Connected);
            return;
       }
    }
    setError("Invalid login");
}

void CuteboardConnection::handleClipboardCommand(QString data)
{
    emit clipboardData(this,data);
}

QPair<QString, QString> CuteboardConnection::readLine()
{
    int idx = this->readBuffer.indexOf("\r\n");
    QString readLine = QString(this->readBuffer.left(idx));
    this->readBuffer = this->readBuffer.mid(idx+2);

    idx = readLine.indexOf(":");
    if (idx<0) {
        return QPair<QString,QString>("",readLine);
    }
//    D("Read:"<<readLine);
    return QPair<QString,QString>(readLine.left(idx),readLine.mid(idx+2));
}

void CuteboardConnection::handleWatchdogTimeout()
{
    D("Connection timeout.");
    close();
}

void CuteboardConnection::handleReadyToRead()
{
    this->readBuffer.append(this->s->readAll());
//D("Read buffer: "<<this->readBuffer.size()<<", MB:"<<(this->readBuffer.size()/1024/1024));
    if (this->readBuffer.size()>this->maxBufferSize) {
        D("Connection input buffer max size exceeded");
        this->readBuffer.clear();
        close();
    }

    while (this->readBuffer.contains("\r\n")) {
        QPair<QString,QString> line = readLine();
        QString name = line.first;
        QString value = line.second;
        switch(this->state) {
        case New: // Need protocol string....
            if (value=="CB 1.0") {
                setState(PreAuth);
            }
            break;
        case PreAuth:
            if (name=="User") {
                if (value.isEmpty()) {
                    setError("Username needed");
                }
                this->user = value;
                handleLogin();
            } else if (name=="Client") {
                if (value.isEmpty()) {
                    setError("Client info needed");
                }
                this->client = value;
                D("Client: "<<this->client);
                handleLogin();
            } else {
                setError("User and client expected");
            }
            break;
        case AuthChallenge:
            if (name=="ChallengeResponse") {
                handleChallengeResponse(value);
            }
            break;
        case Connected :
            if (name=="Clipboard") {
                handleClipboardCommand(value);
            } else if (name=="Ping") {
                D("Client ping.");
                this->watchdog.start(600000);
                write("Pong");
            }
            break;
        default:
            D("Didn't understand:"<<name<<value.left(255));
            setError("What????");
        }
    }
}

void CuteboardConnection::handleClipboardData(CuteboardConnection *source, QString data)
{
    if (this!=source) {
        D("Got incoming clipboard.");
        write(QString("Clipboard: %1").arg(data));
    }
}
