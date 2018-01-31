#include "cuteboarddclient.h"
#include <QTcpSocket>

#define DEBUG_CLIENT
#ifdef DEBUG_CLIENT
#include <QDebug>
#define D(a) qDebug() <<"[CuteboarddClient]" <<a
#else
#define D(a)
#endif

CuteboarddClient::CuteboarddClient(QObject *parent) : QObject(parent),s(nullptr)
{
    QObject::connect(&this->pingTimer,&QTimer::timeout,this,&CuteboarddClient::handlePingTimeout);
    this->pingTimer.setSingleShot(false);
    this->pingTimer.setInterval(60000); // Ping the server every 60 seconds
}

void CuteboarddClient::connect(QString host, quint16 port)
{
    if (!this->s) {
        QTcpSocket *s;
        this->s = s = new QTcpSocket(this);

        QObject::connect(this->s,&QAbstractSocket::connected,this,&CuteboarddClient::handleConnected);
        QObject::connect(this->s,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,&CuteboarddClient::handleError);
        QObject::connect(this->s,&QAbstractSocket::readyRead,this,&CuteboarddClient::handleReadyRead);

        s->connectToHost(host,port);
    }
}

void CuteboarddClient::write(QString data)
{
    if(this->s) {
        this->s->write(QString("%1\r\n").arg(data).toUtf8());
    }
}

void CuteboarddClient::handleConnected()
{
    write("CB 1.0");//Protocol version
    write("User: me");// User FIX
    write("ChallengeResponse: boobies"); // FIX
    this->pingTimer.start();
}

void CuteboarddClient::handleError(QAbstractSocket::SocketError socketError)
{
    D("An error occured:"<<this->s->errorString());
}

void CuteboarddClient::handlePingTimeout()
{
    write("Ping: ");
}

void CuteboarddClient::handleReadyRead()
{
    qDebug()<<"Server:"<<this->s->readAll();
}
