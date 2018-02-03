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
    connectionId(QUuid::createUuid())
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

void CuteboardConnection::handleLogin(QString user)
{
    this->user = user;
    if (this->user.isEmpty()) {
        setError("Username needed.");
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
    QString readLine = this->s->readLine().trimmed();
    int idx = readLine.indexOf(":");
    if (idx<0) {
        return QPair<QString,QString>("",readLine);
    }
    D("Read:"<<readLine);
    return QPair<QString,QString>(readLine.left(idx),readLine.mid(idx+2));
}

void CuteboardConnection::handleWatchdogTimeout()
{
    D("Connection timeout.");
    close();
}

void CuteboardConnection::handleReadyToRead()
{
    if (!this->s->canReadLine()) {
        return;
    }
    while(this->s->bytesAvailable()>0 && this->s->canReadLine()) {
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
                handleLogin(value);
            } else {
                setError("Ident expected");
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
