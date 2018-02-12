#include "cuteboardserver.h"

#define DEBUG_SERVER
#ifdef DEBUG_SERVER
#include <QDebug>
#define D(a) qDebug() <<"[CuteboardServer]" <<a
#else
#define D(a)
#endif

CuteboardServer::CuteboardServer(QObject *parent) : QObject(parent),hostAddress(""),port(19780)
{
    this->server = new QTcpServer(this);

    connect(this->server,&QTcpServer::newConnection,this,&CuteboardServer::handleIncommingConnection);
}

void CuteboardServer::setListenAddress(QString address)
{
    this->hostAddress = address;
}

void CuteboardServer::setListenPort(quint16 port)
{
    this->port = port;
}

void CuteboardServer::commence()
{
    D("Cuteboard server starting.");
    QHostAddress address(this->hostAddress);
    if (this->hostAddress.isEmpty()) {
        address = QHostAddress::Any;
    }
    this->server->listen(address,this->port);
    D("Server listening on:"<<QString("%1:%2").arg(this->server->serverAddress().toString()).arg(this->server->serverPort())<<", is listening:"<<this->server->isListening());
}

void CuteboardServer::handleIncommingConnection()
{
    D("In handle incoming connection, with"<<this->connections.size()<<"active connections.");
    QTcpSocket *s = this->server->nextPendingConnection();
    if (s) {
        CuteboardConnection *newConnection = new CuteboardConnection(s,this);
        this->connections << newConnection;
        connect(newConnection,&CuteboardConnection::connectionClosed,this,&CuteboardServer::handleConnectionClosed);
    }
}

void CuteboardServer::handleConnectionClosed(CuteboardConnection *connection)
{
    if (this->connections.contains(connection)) {
        D("Removing closed connection.");
        this->connections.removeOne(connection);
        connection->deleteLater();
    }
}
