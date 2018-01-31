#include "cuteboarddclient.h"
#include <QTcpSocket>
#include <QCryptographicHash>

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

void CuteboarddClient::connect(QString host, quint16 port,QString user,QString password)
{
    if (!this->s) {
        QTcpSocket *s;
        this->s = s = new QTcpSocket(this);

        QObject::connect(this->s,&QAbstractSocket::connected,this,&CuteboarddClient::handleConnected);
        QObject::connect(this->s,QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),this,&CuteboarddClient::handleError);
        QObject::connect(this->s,&QAbstractSocket::readyRead,this,&CuteboarddClient::handleReadyRead);

        this->user = user;
        this->password = password;

        s->connectToHost(host,port);
    }
}

void CuteboarddClient::write(QString data)
{
    if(this->s) {
        this->s->write(QString("%1\r\n").arg(data).toUtf8());
    }
}

QPair<QString, QString> CuteboarddClient::readLine()
{
    QString readLine = this->s->readLine().trimmed();
    int idx = readLine.indexOf(":");
    if (idx<0) {
        return QPair<QString,QString>("",readLine);
    }
    D("Read:"<<readLine);
    return QPair<QString,QString>(readLine.left(idx),readLine.mid(idx+2));
}

void CuteboarddClient::handleConnected()
{
    write("CB 1.0");//Protocol version
    write(QString("User: %1").arg(this->user));// User
    //this->pingTimer.start();
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
    QPair<QString,QString> line = readLine();
    QString command = line.first;
    QString value = line.second;

    if (command=="Challenge") {
        D("Got challenge: "<<value);
        QByteArray dataToHash = QString("%1%2").arg(this->password).arg(value).toUtf8();

        write(QString("ChallengeResponse: %1").arg(
                  QString(QCryptographicHash::hash(dataToHash,QCryptographicHash::Sha256).toBase64())
                  ));
    } else if (command=="Pong") {
        D("Pong!");
    }
}
